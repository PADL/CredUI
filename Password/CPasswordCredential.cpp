//
//  CPasswordCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CPasswordCredential.h"

Boolean CPasswordCredential::initWithAttributes(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFTypeRef defaultUsername = NULL;
    CUIFieldRef fields[4] = { 0 };
    
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
    }
    
    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, CFSTR("Password Credential"), NULL, NULL);
    
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassEditText, CFSTR("Username"), defaultUsername,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   if (value && CFStringGetLength((CFStringRef)value)) {
                                       CFDictionarySetValue(_attributes, kGSSAttrNameType, kGSSAttrNameTypeGSSUsername);
                                       CFDictionarySetValue(_attributes, kGSSAttrName, value);
                                   }
    });
    
    fields[2] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), NULL,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   if (value && CFStringGetLength((CFStringRef)value)) {
                                       CFDictionarySetValue(_attributes, kGSSAttrCredentialPassword, value);
                                   }
    });
    
    fields[3] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, CFSTR("Submit"), NULL, NULL);
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    if (attributes)
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
    else
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    // delete any existing cached password, because GSSItem won't be able to acquire a credential otherwise
    CFDictionaryRemoveValue(_attributes, kGSSAttrCredentialPassword);

    return !!_attributes;
}

const CFStringRef CPasswordCredential::getCredentialStatus(void)
{
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kGSSAttrName);
    CFStringRef password = (CFStringRef)CFDictionaryGetValue(_attributes, kGSSAttrCredentialPassword);
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