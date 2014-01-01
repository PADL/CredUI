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
    if (controller->_gssTargetName)
        CFRelease(controller->_gssTargetName);
    
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

    controller->_providers = CUIProvidersCreate(allocator, controller);
    if (controller->_providers == NULL) {
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
__CUIControllerEnumerateCredentialForProviderWithAttributes(CUIControllerRef controller,
                                                            CUIProvider *provider,
                                                            CFDictionaryRef attributes,
                                                            void (^cb)(CUICredentialRef, CFErrorRef))
{
    CUICredentialContext *attrCredContext;
    CUICredentialRef cred = NULL;
    CFErrorRef error = NULL;
    
    attrCredContext = provider->createCredentialWithAttributes(attributes, &error);
    if (attrCredContext)
        cred = CUICredentialCreate(CFGetAllocator(controller), attrCredContext);
    
    if (cred || error)
        cb(cred, error);
    
    if (cred)
        CFRelease(cred);
    if (error)
        CFRelease(error);
    if (attrCredContext)
        attrCredContext->Release();
    
    return !!cred;
}

struct __CUIEnumerateCredentialContext {
    CUIControllerRef controller;
    CUIProvider *provider;
    void (^callback)(CUICredentialRef, CFErrorRef);
    Boolean didEnumerate;
};

static void
__CUIEnumerateOtherCredentialsForProviderCallback(const void *value, void *_context)
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
__CUIControllerEnumerateOtherCredentialsForProvider(CUIControllerRef controller,
                                                    CUIProvider *provider,
                                                    void (^cb)(CUICredentialRef, CFErrorRef))
{
    CFArrayRef otherCredContexts;
    CFErrorRef error = NULL;
    
    __CUIEnumerateCredentialContext enumContext = {
        .controller = controller,
        .provider = provider,
        .callback = cb,
        .didEnumerate = false
    };
    
    otherCredContexts = provider->createOtherCredentials(&error);
    if (otherCredContexts) {
        CFArrayApplyFunction(otherCredContexts,
                             CFRangeMake(0, CFArrayGetCount(otherCredContexts)),
                             __CUIEnumerateOtherCredentialsForProviderCallback,
                             (void *)&enumContext);
        
        CFRelease(otherCredContexts);
    } else if (error) {
        cb(NULL, error);
        CFRelease(error);
    }
    
    return enumContext.didEnumerate;
}

static void
__CUITransformItemAttributeKeys(const void *key, const void *value, void *context)
{
    CFMutableStringRef transformedKey = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, (CFStringRef)key);

    /* XXX this is pretty inefficient because we do it every time */
    if (transformedKey) {
        CFStringFindAndReplace(transformedKey, CFSTR("kGSS"), CFSTR("kCUI"), CFRangeMake(0, 4), 0);
        CFDictionarySetValue((CFMutableDictionaryRef)context, transformedKey, value);
        CFRelease(transformedKey);
    }
}

static void
__CUIEnumerateItemCredentialsCallback(const void *value, void *_context)
{
    GSSItemRef item = (GSSItemRef)value;
    __CUIEnumerateCredentialContext *enumContext = (__CUIEnumerateCredentialContext *)_context;
    CFDictionaryRef transformedAttrs;

    transformedAttrs = CFDictionaryCreateMutable(CFGetAllocator(enumContext->controller),
                                                 0,
                                                 &kCFTypeDictionaryKeyCallBacks,
                                                 &kCFTypeDictionaryValueCallBacks);
    CFDictionaryApplyFunction(item->keys, __CUITransformItemAttributeKeys, (void *)transformedAttrs);

    enumContext->didEnumerate |=
        __CUIControllerEnumerateCredentialForProviderWithAttributes(enumContext->controller,
                                                                    enumContext->provider,
                                                                    transformedAttrs,
                                                                    ^(CUICredentialRef cred, CFErrorRef err) {
                                                                        enumContext->callback(cred, err);
                                                                    });

    CFRelease(transformedAttrs);
}

static Boolean
__CUIControllerEnumerateItemCredentialsForProvider(CUIControllerRef controller,
                                                   CUIProvider *provider,
                                                   CFArrayRef items,
                                                   void (^cb)(CUICredentialRef, CFErrorRef))
{
    __CUIEnumerateCredentialContext enumContext = {
        .controller = controller,
        .provider = provider,
        .callback = cb,
        .didEnumerate = false
    };
    
    if (items) {
        CFArrayApplyFunction(items,
                             CFRangeMake(0, CFArrayGetCount(items)),
                             __CUIEnumerateItemCredentialsCallback,
                             (void *)&enumContext);
    }
    
    return enumContext.didEnumerate;
}

static Boolean
__CUIControllerEnumerateCredentialsForProvider(CUIControllerRef controller,
                                               CUIProvider *provider,
                                               CFArrayRef items,
                                               void (^cb)(CUICredentialRef, CFErrorRef))
{
    Boolean didEnumerate = false;
    
    if (controller->_attributes) {
        didEnumerate |= __CUIControllerEnumerateCredentialForProviderWithAttributes(controller,
                                                                                    provider,
                                                                                    controller->_attributes,
                                                                                    cb);
    }
    
    if ((controller->_usageFlags & kCUIUsageFlagsInCredOnly) == 0) {
        if ((controller->_usage == kCUIUsageScenarioNetwork) &&
            (controller->_usageFlags & kCUIUsageFlagsExcludePersistedCreds) == 0) {
            didEnumerate |= __CUIControllerEnumerateItemCredentialsForProvider(controller,
                                                                               provider,
                                                                               items,
                                                                               cb);
        } else if (controller->_usage == kCUIUsageScenarioLogin) {
            // Here we may enumerate local accounts for example
        }
        
        didEnumerate |= __CUIControllerEnumerateOtherCredentialsForProvider(controller,
                                                                            provider,
                                                                            cb);
    }
    
    return didEnumerate;
}


CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef, CFErrorRef))
{
    CFIndex index;
    CFArrayRef items = NULL;
    CFErrorRef error = NULL;
    Boolean didEnumerate = false;
    
    if ((controller->_usageFlags & kCUIUsageFlagsInCredOnly) == 0 &&
        (controller->_usageFlags & kCUIUsageFlagsExcludePersistedCreds) == 0)
        items = GSSItemCopyMatching(controller->_attributes, &error);
    
    for (index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        didEnumerate |= __CUIControllerEnumerateCredentialsForProvider(controller,
                                                                       (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index),
                                                                       items,
                                                                       cb);
    }

    if (items)
        CFRelease(items);
    else if (error)
        CFRelease(error);
    
    return didEnumerate;
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
CUIControllerSetGssTargetName(CUIControllerRef controller, gss_name_t targetName)
{
    __CUISetter((CFTypeRef &)controller->_gssTargetName, (CFTypeRef)targetName);
}

CUI_EXPORT gss_name_t
CUIControllerGetGssTargetName(CUIControllerRef controller)
{
    return controller->_gssTargetName;
}
