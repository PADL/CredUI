//
//  CUIController.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static CFTypeID _CUIControllerTypeID = _kCFRuntimeNotATypeID;

static void _CUIControllerDeallocate(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    if (controller->_providers)
        CFRelease(controller->_providers);
    if (controller->_attributes)
        CFRelease(controller->_attributes);
    if (controller->_authError)
        CFRelease(controller->_authError);
    if (controller->_gssContextHandle)
        CFRelease(controller->_gssContextHandle);
    if (controller->_targetName)
        CFRelease(controller->_targetName);
    
}
static CFStringRef _CUIControllerCopyDescription(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    return CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                    CFSTR("<CUIController %p>{usage = %u, usageFlags = %08x}"),
                                    controller, (unsigned)controller->_usage, (unsigned)controller->_usageFlags);
}

static const CFRuntimeClass _CUIControllerClass = {
    0,
    "CUICredential",
    NULL, // init
    NULL, // copy
    _CUIControllerDeallocate,
    NULL,
    NULL, // _CUICredentialHash,
    NULL, // _CUICredentialCopyFormattingDesc
    _CUIControllerCopyDescription
};

CUI_EXPORT CFTypeID
CUIControllerGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (_CUIControllerTypeID == _kCFRuntimeNotATypeID) {
            _CUIControllerTypeID = _CFRuntimeRegisterClass(&_CUIControllerClass);
        }
    });
    
    return _CUIControllerTypeID;
}

CUI_EXPORT CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usage,
                    CUIUsageFlags usageFlags)
{
    CUIControllerRef controller;
    
    controller = (CUIControllerRef)_CFRuntimeCreateInstance(allocator, CUIControllerGetTypeID(), sizeof(struct __CUIController) - sizeof(CFRuntimeBase), NULL);
    if (controller == NULL)
        return NULL;
    
    controller->_usage = usage;
    controller->_usageFlags = usageFlags;

    if (!CUIProvidersCreate(allocator, controller)) {
        CFRelease(controller);
        return NULL;
    }
    
    controller->_attributes = CFDictionaryCreateMutable(allocator,
                                                        0,
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
    if (controller->_attributes == NULL) {
        CFRelease(controller);
        return NULL;
    }
    
    return controller;
}

struct __CUIEnumerateCredentialContext {
    CUIControllerRef controller;
    CUIProvider *provider;
    void (^callback)(CUICredentialRef, CFErrorRef);
    Boolean didEnumerate;
};

static void
__CUIEnumerateMatchingCredentialsForProviderCallback(const void *value, void *_context)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    __CUIEnumerateCredentialContext *enumContext = (__CUIEnumerateCredentialContext *)_context;
    CUICredentialRef cred = CUICredentialCreate(CFGetAllocator(enumContext->controller), credContext);
    
    if (cred) {
        enumContext->callback(cred, NULL);
        CFRelease(cred);
        enumContext->didEnumerate = true;
    }
}

static Boolean
__CUIControllerEnumerateMatchingCredentialsForProvider(CUIControllerRef controller,
                                                       CFDictionaryRef attributes,
                                                       CUIProvider *provider,
                                                       void (^cb)(CUICredentialRef, CFErrorRef))
{
    CFArrayRef matchingCredContexts;
    CFErrorRef error = NULL;
    
    __CUIEnumerateCredentialContext enumContext = {
        .controller = controller,
        .provider = provider,
        .callback = cb,
        .didEnumerate = false
    };
    
    matchingCredContexts = provider->copyMatchingCredentials(attributes, &error);
    if (matchingCredContexts) {
        CFArrayApplyFunction(matchingCredContexts,
                             CFRangeMake(0, CFArrayGetCount(matchingCredContexts)),
                             __CUIEnumerateMatchingCredentialsForProviderCallback,
                             (void *)&enumContext);
        
        CFRelease(matchingCredContexts);
    } else if (error) {
        cb(NULL, error);
        CFRelease(error);
    }
    
    return enumContext.didEnumerate;
}

CUI_EXPORT Boolean
__CUIControllerEnumerateCredentialsExcepting(CUIControllerRef controller,
                                             CFDictionaryRef attributes,
                                             CFTypeRef notFactories,
                                             void (^cb)(CUICredentialRef, CFErrorRef))
{
    CFArrayRef items = NULL;
    CFErrorRef error = NULL;
    Boolean didEnumerate = false;
    
    for (CFIndex index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        CFUUIDRef factory = (CFUUIDRef)CFArrayGetValueAtIndex(controller->_factories, index);
        CUIProvider *provider = (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);
        Boolean skipThisProvider = false;
        
        if (notFactories) {
            if (CFGetTypeID(notFactories) == CFArrayGetTypeID())
                skipThisProvider = CFArrayContainsValue((CFArrayRef)notFactories,
                                                        CFRangeMake(0, CFArrayGetCount((CFArrayRef)notFactories)),
                                                        (void *)factory);
            else
                skipThisProvider = CFEqual(notFactories, factory);
        }
        
        if (skipThisProvider)
            continue;
        
        didEnumerate |= __CUIControllerEnumerateMatchingCredentialsForProvider(controller,
                                                                               attributes,
                                                                               provider,
                                                                               cb);
    }

    if (items)
        CFRelease(items);
    else if (error)
        CFRelease(error);
    
    return didEnumerate;
}

CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef, CFErrorRef))
{
    return __CUIControllerEnumerateCredentialsExcepting(controller, controller->_attributes, NULL, cb);
}

static void
__CUICopyMutableAttributesKeys(const void *key, const void *value, void *context)
{
    // XXX probably should filter these
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}

CUI_EXPORT void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef attributes)
{
    CFDictionaryApplyFunction(attributes, __CUICopyMutableAttributesKeys, (void *)controller->_attributes);
}

CUI_EXPORT CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller)
{
    return controller->_attributes;
}

CUI_EXPORT void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError)
{
    __CUISetter((CFTypeRef &)controller->_authError, authError);
}

CUI_EXPORT CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller)
{
    return controller->_authError;
}

CUI_EXPORT Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller,
                              CUICredUIContextProperties whichProps,
                              const CUICredUIContext *uic)
{
    CUICredUIContext *cuic = &controller->_uiContext;
    
    if (uic->version != 0)
        return false;
    
    if (whichProps & kCUICredUIContextPropertyParentWindow)
        __CUISetter(cuic->parentWindow, uic->parentWindow);
    if (whichProps & kCUICredUIContextPropertyMessageText)
        __CUISetter((CFTypeRef &)cuic->messageText, (CFTypeRef)uic->messageText);
    if (whichProps & kCUICredUIContextPropertyTitleText)
        __CUISetter((CFTypeRef &)cuic->titleText, (CFTypeRef)uic->titleText);
    
    return true;
}

CUI_EXPORT const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller)
{
    return &controller->_uiContext;
}

CUI_EXPORT void
CUIControllerSetGssContextHandle(CUIControllerRef controller, CFTypeRef gssContextHandle)
{
    __CUISetter(controller->_gssContextHandle, gssContextHandle);
}

CUI_EXPORT CFTypeRef
CUIControllerGetGssContextHandle(CUIControllerRef controller)
{
    return controller->_gssContextHandle;
}

CUI_EXPORT void
CUIControllerSetTargetName(CUIControllerRef controller, CFTypeRef targetName)
{
    __CUISetter((CFTypeRef &)controller->_targetName, targetName);
}

CUI_EXPORT CFTypeRef
CUIControllerGetTargetName(CUIControllerRef controller)
{
    return controller->_targetName;
}
