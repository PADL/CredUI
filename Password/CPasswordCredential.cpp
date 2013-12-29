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
    CFTypeRef defaultUsername = NULL;
    CFTypeRef defaultPassword = NULL;
    CUIFieldRef fields[3] = { 0 };
    
    if (authIdentity) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(authIdentity, kGSSAttrNameType);
        
        if (nameType && CFEqual(nameType, kGSSAttrNameTypeGSSUsername))
            defaultUsername = CFDictionaryGetValue(authIdentity, kGSSAttrName);
        
        defaultPassword = CFDictionaryGetValue(authIdentity, kGSSAttrCredentialPassword);
    }
    
    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, CFSTR("Password Credential"), NULL, NULL);
    
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassEditText, CFSTR("Username"), defaultUsername,
                               ^(CUIFieldRef field, CFTypeRef value) {
        CFDictionarySetValue(_authIdentity, kGSSAttrNameType, kGSSAttrNameTypeGSSUsername);
        CFDictionarySetValue(_authIdentity, kGSSAttrName, value);
    });
    
    fields[2] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), defaultPassword,
                               ^(CUIFieldRef field, CFTypeRef value) {
        CFDictionarySetValue(_authIdentity, kGSSAttrCredentialPassword, value);
    });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    if (authIdentity)
        _authIdentity = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, authIdentity);
    else
        _authIdentity = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    return !!_authIdentity;
}
