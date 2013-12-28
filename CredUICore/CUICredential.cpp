
//
//  CUICredential.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore.h"

struct __CUICredential {
    CFRuntimeBase _base;
    CUICredentialContext *_context;
};

static CFTypeID _CUICredentialTypeID = _kCFRuntimeNotATypeID;

static void _CUICredentialDeallocate(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;

    if (cred->_context)
        cred->_context->Release();
}

static Boolean _CUICredentialEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    return (cf1 == cf2);
}

static CFStringRef _CUICredentialCopyDescription(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;
    CFStringRef desc = NULL;
  
    if (cred->_context)
        desc = cred->_context->copyDescription();
    
    if (desc == NULL) {
        desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                        CFSTR("<CUICredential %p>"), cred);
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
CUICredentialCreate(CFAllocatorRef allocator, CUICredentialContext *context)
{
    CUICredentialRef cred;
    
    cred = (CUICredentialRef)_CFRuntimeCreateInstance(allocator, CUICredentialGetTypeID(), sizeof(struct __CUICredential) - sizeof(CFRuntimeBase), NULL);
    if (cred == NULL)
        return NULL;
    
    context->AddRef();
    cred->_context = context;

    return cred;
}

CFArrayRef
CUICredentialGetFields(CUICredentialRef cred)
{
    if (cred->_context)
        return cred->_context->getFields();
    
    return NULL;
}