//
//  SampleCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <Security/Security.h>

#include "SampleCredential.h"
#include "CUIProviderUtilities.h"
#include "CustomField.h"

Boolean SampleCredential::initWithControllerAndAttributes(CUIControllerRef controller,
                                                          CUIUsageFlags usageFlags,
                                                          CFDictionaryRef attributes,
                                                          CFErrorRef *error)
{
    CUIFieldRef fields[4] = { 0 };
    size_t cFields = 0;
    
    if (error != NULL)
        *error = NULL;
    
    if (attributes) {
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    /*
     * Set some default attributes for this credential.
     */
    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, CFSTR("SampleCredentialProvider"));
    if (usageFlags & kCUIUsageFlagsGeneric)
        CFDictionarySetValue(_attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    
    /*
     * This is the title field that identifies the provider.
     */
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Sample Credential"), NULL);
    
    /*
     * The username field.
     */
    fields[cFields++] = CustomFieldCreate(_attributes);
    
    /*
     * The password field.
     */
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassPasswordText, CFSTR("Password"), NULL,
                                       ^(CUIFieldRef field, CFTypeRef value) {
                                           if (value) {
                                               CFDictionarySetValue(_attributes, kCUIAttrCredentialPassword, value);
                                           } else {
                                               CFDictionaryRemoveValue(_attributes, kCUIAttrCredentialPassword);
                                           }
                                       });
    
    /*
     * The "submit button". This is called when the credential is about to be submitted.
     */
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, NULL, NULL,
                                       ^(CUIFieldRef field, CFTypeRef value) {
                                       });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, cFields, &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    return true;
}

/*
 * Determine whether the credential is able to be submitted.
 */
const CFStringRef SampleCredential::getCredentialStatus(void)
{
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrName);
    
    if (username == NULL || CFStringGetLength(username) == 0)
        return kCUICredentialNotFinished;
    
    CFTypeRef password = CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);
    if (password &&
        (CFGetTypeID(password) == CFStringGetTypeID() && CFStringGetLength((CFStringRef)password)))
        return kCUICredentialReturnCredentialFinished;
    else
        return kCUICredentialNotFinished;
}

