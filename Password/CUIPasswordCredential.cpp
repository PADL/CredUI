//
//  CUIPasswordCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPasswordCredential.h"

Boolean CUIPasswordCredential::initWithAttributes(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFTypeRef defaultUsername = NULL;
    CUIFieldRef fields[3] = { 0 };
    
    *error = NULL;
    
    if (attributes) {
        /*
         * If this is a stored item (i.e. it has a UUID) and it's not a password credential, ignore it
         */
        if (CFDictionaryGetValue(attributes, kGSSAttrUUID) &&
            !CFDictionaryGetValue(attributes, kGSSAttrCredentialPassword))
            return false;

        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kGSSAttrNameType);
        
        if (nameType && CFEqual(nameType, kGSSAttrNameTypeGSSUsername))
            defaultUsername = CFDictionaryGetValue(attributes, kGSSAttrName);
        
        if (defaultUsername &&
            CFDictionaryGetValue(attributes, kGSSAttrCredentialPassword))
            _inCredUsable = true;
        
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
        // delete any existing cached password, because GSSItem won't be able to acquire a credential otherwise
        CFDictionaryRemoveValue(_attributes, kGSSAttrCredentialPassword);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, CFSTR("Password Credential"), NULL, NULL);
    
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassEditText, CFSTR("Username"), defaultUsername,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   CFDictionarySetValue(_attributes, kGSSAttrNameType, kGSSAttrNameTypeGSSUsername);
                                   if (value) {
                                       CFDictionarySetValue(_attributes, kGSSAttrName, value);
                                   } else {
                                       CFDictionaryRemoveValue(_attributes, kGSSAttrName);
                                   }
    });
    
    fields[2] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), NULL,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   if (value) {
                                       CFDictionarySetValue(_attributes, kGSSAttrCredentialPassword, value);
                                   } else {
                                       CFDictionaryRemoveValue(_attributes, kGSSAttrCredentialPassword);
                                   }
    });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    return true;
}

const CFStringRef CUIPasswordCredential::getCredentialStatus(void)
{
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kGSSAttrName);
    CFStringRef password = (CFStringRef)CFDictionaryGetValue(_attributes, kGSSAttrCredentialPassword);
    CFStringRef status;
    
    fprintf(stderr, "attr dict %p", _attributes);
    if (_inCredUsable) {
        status = kCUICredentialReturnCredentialFinished;
    } else if ((username && CFStringGetLength(username)) &&
        (password && CFStringGetLength(password))) {
        status = kCUICredentialReturnCredentialFinished;
    } else {
        status = kCUICredentialNotFinished;
    }
    
    return status;
}
