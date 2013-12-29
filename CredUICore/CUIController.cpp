//
//  CUIController.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#include <GSS/GSS.h>

struct __CUIController {
    CFRuntimeBase _base;
    CFArrayRef _providers;
    CUIUsageScenario _usage;
    CUIUsageFlags _usageFlags;
    CUICredUIContext _uiContext;
    CFMutableDictionaryRef _authIdentity;
    CFErrorRef _authError;
    CFIndex _flags;
    CFTypeRef _gssContextHandle; // for use with GSSKit/NegoEx
    gss_name_t _gssTargetName;
    
};

static CFTypeID _CUIControllerTypeID = _kCFRuntimeNotATypeID;

static void _CUIControllerDeallocate(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    if (controller->_providers)
        CFRelease(controller->_providers);
    if (controller->_authIdentity)
        CFRelease(controller->_authIdentity);
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

CFTypeID
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

CUIControllerRef
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
    
    controller->_authIdentity = CFDictionaryCreateMutable(allocator,
                                                          0,
                                                          &kCFTypeDictionaryKeyCallBacks,
                                                          &kCFTypeDictionaryValueCallBacks);
    if (controller->_authIdentity == NULL) {
        CFRelease(controller);
        return NULL;
    }
    
    return controller;
}

static Boolean
CUIControllerEnumerateCredentialsForProvider(CUIControllerRef controller, CUIProvider *provider, void (^cb)(CUICredentialRef))
{
    CUICredentialContext *authIdentityCredContext;
    CFArrayRef otherCredContexts;
    CFIndex index;
    
    authIdentityCredContext = provider->getCredentialForAuthIdentity(controller->_authIdentity);
    if (authIdentityCredContext) {
        CUICredentialRef cred = CUICredentialCreate(CFGetAllocator(controller), authIdentityCredContext);
        if (cred == NULL)
            return false;
        
        cb(cred);
        CFRelease(cred);
    }
    
    otherCredContexts = provider->getOtherCredentials();
    if (otherCredContexts) {
        for (index = 0; index < CFArrayGetCount(otherCredContexts); index++) {
            CUICredentialRef cred = CUICredentialCreate(CFGetAllocator(controller),
                                                        (CUICredentialContext *)CFArrayGetValueAtIndex(otherCredContexts, index));
            if (cred == NULL)
                return false;
            
            cb(cred);
            CFRelease(cred);
        }
    }
    
    return true;
}

Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef))
{
    CFIndex index;
    Boolean didEnumerate = false;
    
    for (index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        didEnumerate |= CUIControllerEnumerateCredentialsForProvider(controller,
                                                                     (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index),
                                                                     cb);
    }
    
    return didEnumerate;
}

static void
__CUICopyMutableAuthIdentityKeys(const void *key, const void *value, void *context)
{
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}

void
CUIControllerSetAuthIdentity(CUIControllerRef controller, CFDictionaryRef authIdentity)
{
    CFDictionaryApplyFunction(controller->_authIdentity, __CUICopyMutableAuthIdentityKeys, (void *)authIdentity);
}


CFDictionaryRef
CUIControllerGetAuthIdentity(CUIControllerRef controller)
{
    return controller->_authIdentity;
}

void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError)
{
    __CUISetter((CFTypeRef &)controller->_authError, authError);
}

CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller)
{
    return controller->_authError;
}

Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller, const CUICredUIContext *uic)
{
    CUICredUIContext *cuic = &controller->_uiContext;
    
    if (uic->version != 0)
        return false;
    
    __CUISetter(cuic->parentWindow, uic->parentWindow);
    __CUISetter((CFTypeRef &)cuic->messageText, (CFTypeRef)uic->messageText);
    __CUISetter((CFTypeRef &)cuic->titleText, (CFTypeRef)uic->titleText);
    
    return true;
}

const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller)
{
    return &controller->_uiContext;
}

void
CUIControllerSetSaveToKeychain(CUIControllerRef controller, Boolean save)
{
    CFBooleanRef value = save ? kCFBooleanTrue : kCFBooleanFalse;
    CFDictionarySetValue(controller->_authIdentity, kGSSAttrStatusPersistant, value);
}

Boolean
CUIControllerGetSaveToKeychain(CUIControllerRef controller)
{
    return CFBooleanGetValue((CFBooleanRef)CFDictionaryGetValue(controller->_authIdentity, kGSSAttrStatusPersistant));
}

void
CUIControllerSetGssContextHandle(CUIControllerRef controller, CFTypeRef gssContextHandle)
{
    __CUISetter(controller->_gssContextHandle, gssContextHandle);
}

CFTypeRef
CUIControllerGetGssContextHandle(CUIControllerRef controller)
{
    return controller->_gssContextHandle;
}

void
CUIControllerSetGssTargetName(CUIControllerRef controller, gss_name_t targetName)
{
    __CUISetter((CFTypeRef &)controller->_gssTargetName, (CFTypeRef)targetName);
}

gss_name_t
CUIControllerGetGssTargetName(CUIControllerRef controller)
{
    return controller->_gssTargetName;
}