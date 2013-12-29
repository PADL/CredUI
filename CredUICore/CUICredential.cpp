
//
//  CUICredential.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static const void *
_CUICredentialContextRetain(CFAllocatorRef allocator, const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    credContext->AddRef();
    return credContext;
}

static void
_CUICredentialContextRelease(CFAllocatorRef allocator, const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    credContext->Release();
}

static CFStringRef
_CUICredentialContextCopyDescription(const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    return credContext->copyDescription();
}

static Boolean
_CUICredentialContextEqual(const void *value1, const void *value2)
{
    CUICredentialContext *credContext1 = (CUICredentialContext *)value1;
    CUICredentialContext *credContext2 = (CUICredentialContext *)value2;
    
    return (credContext1 == credContext2);
}

CFArrayCallBacks kCUICredentialContextArrayCallBacks = {
    .version = 0,
    .retain = _CUICredentialContextRetain,
    .release = _CUICredentialContextRelease,
    .copyDescription = _CUICredentialContextCopyDescription,
    .equal = _CUICredentialContextEqual
};

struct __CUICredential {
    CFRuntimeBase _base;
    CUICredentialContext *_context;
};

static CFTypeID _CUICredentialTypeID = _kCFRuntimeNotATypeID;

static void _CUICredentialDeallocate(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;

    if (cred->_context)
        _CUICredentialContextRelease(CFGetAllocator(cf), cred->_context);
}

static Boolean _CUICredentialEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    CUICredentialRef cred1 = (CUICredentialRef)cf1;
    CUICredentialRef cred2 = (CUICredentialRef)cf2;
    
    return (cred1 == cred2 ||
            _CUICredentialContextEqual(cred1->_context, cred2->_context));
}

static CFStringRef _CUICredentialCopyDescription(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;
    CFStringRef contextDesc = NULL;
    CFStringRef desc;
  
    if (cred->_context)
        contextDesc = _CUICredentialContextCopyDescription(cred->_context);
    
    if (contextDesc == NULL) {
        desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                        CFSTR("<CUICredential %p>{context = %p}"), cred, cred->_context);
    } else {
        desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                        CFSTR("<CUICredential %p{context = \"%@\"}>"), cred, contextDesc);
    }
    
    return desc;
}

static const CFRuntimeClass _CUICredentialClass = {
    0,
    "CUICredential",
    NULL, // init
    NULL, // copy
    _CUICredentialDeallocate,
    _CUICredentialEqual,
    NULL, // _CUICredentialHash,
    NULL, // _CUICredentialCopyFormattingDesc
    _CUICredentialCopyDescription
};

CFTypeID
CUICredentialGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (_CUICredentialTypeID == _kCFRuntimeNotATypeID) {
            _CUICredentialTypeID = _CFRuntimeRegisterClass(&_CUICredentialClass);
        }
    });
    
    return _CUICredentialTypeID;
}

CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknown *iunk)
{
    CUICredentialRef cred;
    CUICredentialContext *credContext;
    
    cred = (CUICredentialRef)_CFRuntimeCreateInstance(allocator, CUICredentialGetTypeID(), sizeof(struct __CUICredential) - sizeof(CFRuntimeBase), NULL);
    if (cred == NULL)
        return NULL;
    
    iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUICredentialInterfaceID), (void **)&credContext);
    if (credContext == NULL) {
        CFRelease(cred);
        return NULL;
    }
    
    cred->_context = credContext;

    return cred;
}

CFArrayRef
CUICredentialGetFields(CUICredentialRef cred)
{
    if (cred->_context)
        return cred->_context->getFields();
    
    return NULL;
}

extern CFDictionaryRef
CUICredentialGetAuthIdentity(CUICredentialRef cred)
{
    if (cred->_context)
        return cred->_context->getAuthIdentity();
    
    return NULL;
}
