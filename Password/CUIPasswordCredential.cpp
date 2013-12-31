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
        if (CFDictionaryGetValue(attributes, kCUIAttrUUID) &&
            !CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
            return false;

        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = CFDictionaryGetValue(attributes, kCUIAttrName);
        
        if (defaultUsername &&
            CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
            _inCredUsable = true;
        
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
        // delete any existing cached password, because GSSItem won't be able to acquire a credential otherwise
        CFDictionaryRemoveValue(_attributes, kCUIAttrCredentialPassword);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, CFSTR("Password Credential"), NULL, NULL);
    
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassEditText, CFSTR("Username"), defaultUsername,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   CFDictionarySetValue(_attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername);
                                   if (value) {
                                       CFDictionarySetValue(_attributes, kCUIAttrName, value);
                                   } else {
                                       CFDictionaryRemoveValue(_attributes, kCUIAttrName);
                                   }
    });
    
    fields[2] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), NULL,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   if (value) {
                                       CFDictionarySetValue(_attributes, kCUIAttrCredentialPassword, value);
                                   } else {
                                       CFDictionaryRemoveValue(_attributes, kCUIAttrCredentialPassword);
                                   }
    });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    return true;
}

const CFStringRef CUIPasswordCredential::getCredentialStatus(void)
{
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrName);
    CFStringRef password = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);
    CFStringRef status;
    
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
