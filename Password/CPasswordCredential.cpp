//
//  CPasswordCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CPasswordCredential.h"

Boolean CPasswordCredential::initWithAuthIdentity(CFDictionaryRef authIdentity)
{
    _usernameField = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassEditText, CFSTR("Username"), ^(CUIFieldRef field, CFTypeRef value) {
        CFDictionarySetValue(_authIdentity, kGSSAttrNameTypeGSSUsername, value);
    });
    
    if (_usernameField == NULL)
        return false;
    
    _passwordField = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), ^(CUIFieldRef field, CFTypeRef value) {
        CFDictionarySetValue(_authIdentity, kGSSAttrCredentialPassword, value);
    });
    
    if (_passwordField == NULL)
        return false;
    
    const void *values[] = { _usernameField, _passwordField };
    
    _fields = CFArrayCreate(kCFAllocatorDefault, values, 2, &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    if (authIdentity) {
        _authIdentity = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, authIdentity);
        if (_authIdentity == NULL)
            return NULL;
    }
    
    return true;
}

CFDictionaryRef CPasswordCredential::getAuthIdentity(void)
{
    return _authIdentity;
}