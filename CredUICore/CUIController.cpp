//
//  CUIController.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static CFTypeID _CUIControllerTypeID = _kCFRuntimeNotATypeID;

static inline void
_CUISetter(CFTypeRef &dst, CFTypeRef src)
{
    if (dst != src) {
        if (dst) {
            CFRelease(dst);
            dst = NULL;
        }
        if (src) {
            dst = CFRetain(src);
        }
    }
}

static void _CUIControllerDeallocate(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    if (controller->_factories)
        CFRelease(controller->_factories);
    if (controller->_providers)
        CFRelease(controller->_providers);
    if (controller->_uiContext.parentWindow)
        CFRelease(controller->_uiContext.parentWindow);    
    if (controller->_uiContext.messageText)
        CFRelease(controller->_uiContext.messageText);    
    if (controller->_uiContext.titleText)
        CFRelease(controller->_uiContext.titleText);    
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

    if (!_CUIProvidersCreate(allocator, controller)) {
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

static Boolean
_CUIContainsValueP(CFTypeRef cf, CFTypeRef value)
{
    Boolean ret = false;

    if (cf) {
        if (CFGetTypeID(cf) == CFArrayGetTypeID())
            ret = CFArrayContainsValue((CFArrayRef)cf,
                                       CFRangeMake(0, CFArrayGetCount((CFArrayRef)cf)),
                                       (void *)value);
        else
            ret = CFEqual(cf, value);
    }

    return ret;
}

struct CUIEnumerateCredentialContext {
    CUIControllerRef controller;
    CFDictionaryRef attributes;
    CFTypeRef notFactories;
    CFUUIDRef factory;
    CUIProvider *provider;
    void (^callback)(CUICredentialRef, Boolean, CFErrorRef);
    CFIndex index;
    CFIndex defaultCredentialIndex;
    Boolean didEnumerate;
};

static void
_CUIEnumerateMatchingCredentialsForProviderCallback(const void *value, void *_context)
{
    CUIEnumerateCredentialContext *enumContext = (CUIEnumerateCredentialContext *)_context;
    CUICredentialRef cred = (CUICredentialRef)value;

    CFUUIDRef providerFactoryID = (CFUUIDRef)CFDictionaryGetValue(CUICredentialGetAttributes(cred), kCUIAttrProviderFactoryID);
    assert(providerFactoryID == NULL || CFGetTypeID(providerFactoryID) == CFUUIDGetTypeID());

    CFUUIDRef persistenceFactoryID = (CFUUIDRef)CFDictionaryGetValue(CUICredentialGetAttributes(cred), kCUIAttrPersistenceFactoryID);
    assert(persistenceFactoryID == NULL || CFGetTypeID(persistenceFactoryID) == CFUUIDGetTypeID());

    assert(CFEqual(providerFactoryID, enumContext->factory) || CFEqual(persistenceFactoryID, enumContext->factory));
    assert(cred);

    enumContext->callback(cred, (enumContext->index == enumContext->defaultCredentialIndex), NULL);
    enumContext->didEnumerate = true;

    enumContext->index++;
}

static Boolean
_CUIControllerEnumerateMatchingCredentialsForProvider(CUIControllerRef controller,
                                                       CFDictionaryRef attributes,
                                                       CFTypeRef notFactories,
                                                       CFUUIDRef factory,
                                                       CUIProvider *provider,
                                                       void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    CFArrayRef matchingCreds;
    CFErrorRef error = NULL;

    CUIEnumerateCredentialContext enumContext = {
        .controller = controller,
        .attributes = attributes,
        .notFactories = notFactories,
        .factory = factory,
        .provider = provider,
        .callback = cb,
        .didEnumerate = false,
        .index = 0,
        .defaultCredentialIndex = kCFNotFound
    };
    
    matchingCreds = provider->copyMatchingCredentials(attributes, &enumContext.defaultCredentialIndex, &error);
    if (matchingCreds) {
        CFArrayApplyFunction(matchingCreds,
                             CFRangeMake(0, CFArrayGetCount(matchingCreds)),
                             _CUIEnumerateMatchingCredentialsForProviderCallback,
                             (void *)&enumContext);
        
        CFRelease(matchingCreds);
    } else if (error) {
        cb(NULL, kCFNotFound, error);
        CFRelease(error);
    }
    
    return enumContext.didEnumerate;
}

CUI_EXPORT Boolean
_CUIControllerEnumerateCredentialsExcepting(CUIControllerRef controller,
                                            CFDictionaryRef attributes,
                                            CFTypeRef notFactories,
                                            void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    CFArrayRef items = NULL;
    CFErrorRef error = NULL;
    Boolean didEnumerate = false;
    
    for (CFIndex index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        CFUUIDRef factory = (CFUUIDRef)CFArrayGetValueAtIndex(controller->_factories, index);
        CUIProvider *provider = (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);
        Boolean skipThisProvider = _CUIContainsValueP(notFactories, factory);
        
        if (skipThisProvider)
            continue;
        
        didEnumerate |= _CUIControllerEnumerateMatchingCredentialsForProvider(controller,
                                                                              attributes,
                                                                              notFactories,
                                                                              factory,
                                                                              provider,
                                                                              cb);
    }

    if (items)
        CFRelease(items);
    else if (error)
        CFRelease(error);
    
    return didEnumerate;
}

static CFDictionaryRef
_CUIControllerCreateAttributesAdjustedForAuthError(CUIControllerRef controller)
{
    /*
     * This is to handle a special case which is not yet supported by Heimdal,
     * that is when a mechanism is in play to successfully authenticate but
     * needs more information from the user. In this case it will return
     * GSS_S_CONTINUE_NEEDED | GSS_S_PROMPTING_NEEDED.
     */
    if (controller->_gssContextHandle != GSS_C_NO_CONTEXT &&
        GSSIsPromptingNeeded(controller->_authError) &&
        !GSS_ERROR(CFErrorGetCode(controller->_authError))) {
        CFStringRef attrClass;
        
        attrClass = _CUICopyAttrClassForAuthError(controller->_authError);
        if (attrClass) {
            CFMutableDictionaryRef attributes;
            
            if (controller->_attributes)
                attributes = CFDictionaryCreateMutableCopy(CFGetAllocator(controller), 0, controller->_attributes);
            else
                attributes = CFDictionaryCreateMutable(CFGetAllocator(controller),
                                                       0,
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
            if (attributes)
                CFDictionarySetValue(attributes, kCUIAttrClass, attrClass);

            CFRelease(attrClass);

            return attributes;
        }
    }
    
    return controller->_attributes ? (CFDictionaryRef)CFRetain(controller->_attributes) : NULL;
}

CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    CFDictionaryRef attributes = _CUIControllerCreateAttributesAdjustedForAuthError(controller);
    Boolean ret;
    
    ret = _CUIControllerEnumerateCredentialsExcepting(controller, attributes, NULL, cb);
    
    if (attributes)
        CFRelease(attributes);

    return ret;
}

static void
_CUICopyMutableAttributesKeys(const void *key, const void *value, void *context)
{
    // XXX probably should filter these
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}

CUI_EXPORT void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef attributes)
{
    CFDictionaryApplyFunction(attributes, _CUICopyMutableAttributesKeys, (void *)controller->_attributes);
}

CUI_EXPORT CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller)
{
    return controller->_attributes;
}

CUI_EXPORT void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError)
{
    _CUISetter((CFTypeRef &)controller->_authError, authError);
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
        _CUISetter(cuic->parentWindow, uic->parentWindow);
    if (whichProps & kCUICredUIContextPropertyMessageText)
        _CUISetter((CFTypeRef &)cuic->messageText, (CFTypeRef)uic->messageText);
    if (whichProps & kCUICredUIContextPropertyTitleText)
        _CUISetter((CFTypeRef &)cuic->titleText, (CFTypeRef)uic->titleText);
    
    return true;
}

CUI_EXPORT const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller)
{
    return &controller->_uiContext;
}

CUI_EXPORT void
CUIControllerSetGSSContextHandle(CUIControllerRef controller, CFTypeRef gssContextHandle)
{
    _CUISetter(controller->_gssContextHandle, gssContextHandle);
}

CUI_EXPORT CFTypeRef
CUIControllerGetGSSContextHandle(CUIControllerRef controller)
{
    return controller->_gssContextHandle;
}

CUI_EXPORT void
CUIControllerSetTargetName(CUIControllerRef controller, CFTypeRef targetName)
{
    _CUISetter((CFTypeRef &)controller->_targetName, targetName);
}

CUI_EXPORT CFTypeRef
CUIControllerGetTargetName(CUIControllerRef controller)
{
    return controller->_targetName;
}

CUI_EXPORT CFStringRef
CUICopyTargetDisplayName(CFTypeRef targetName)
{
    CFStringRef displayName = NULL;
    CFTypeID targetNameTypeID;
    
    if (targetName == NULL)
        return NULL;
    
    targetNameTypeID = CFGetTypeID(targetName);
    if (targetNameTypeID == CFURLGetTypeID()) {
        displayName = (CFStringRef)CFRetain(CFURLGetString((CFURLRef)targetName));
    } else if (targetNameTypeID == CFStringGetTypeID()) {
        displayName = (CFStringRef)CFRetain(targetName);
    } else {
        /* here's hoping it's a GSS name, because otherwise we will crash XXX */
        displayName = GSSNameCreateDisplayString((gss_name_t)targetName);
    }
    
    return displayName;
}

CUI_EXPORT CFStringRef
CUICopyTargetHostName(CFTypeRef targetName)
{
    CFStringRef hostName = NULL;
    
    if (targetName == NULL)
        return NULL;
    
    if (CFGetTypeID(targetName) == CFURLGetTypeID()) {
        hostName = CFURLCopyHostName((CFURLRef)targetName);
    } else {
        // We need to do something useful for GSS service names
    }
    
    return hostName;
}
