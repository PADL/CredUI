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

/*
 * Get the default username for this credential
 */
CFStringRef SampleCredential::getDefaultUsername(void)
{
    CFStringRef defaultUsername;

    defaultUsername = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrNameDisplay);
    if (defaultUsername == NULL) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrName);
    }
    
    return defaultUsername;
}

/*
 * Set the username on this credential
 */
void SampleCredential::setUsername(CFStringRef username)
{
    if (username) {
        CFDictionarySetValue(_attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername);
        CFDictionarySetValue(_attributes, kCUIAttrName, CFStringCreateCopy(CFGetAllocator(_attributes), username));
    } else {
        CFDictionaryRemoveValue(_attributes, kCUIAttrName);
    }
}

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
     * The username field. Normally you would just use CUIFieldCreate(kCUIFieldClassEditText) with a
     * block that would set the username in the credential dictionary (e.g. call this->setUsername()).
     * Instead here we create a custom subclass of CUIField.
     *
     * Note: this isn't recommended because it introduces a dependency on Cocoa into CredUICore which
     * eliminates the possibility of this provider being used in a command-line application.
     */
    fields[cFields++] = CustomFieldCreate(this);
    
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
     * The "submit button". This is called when the credential is about to be submitted. It is
     * the equivalent of having a willSubmit method; note that, unlike the other fields, CredUI
     * may not render individual submit buttons for each provider, but rather a single one for
     * all providers. The callback block will be invoked if and when this credential is submitted.
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

    /* Do we have a username? */    
    if (username == NULL || CFStringGetLength(username) == 0)
        return kCUICredentialNotFinished;

    /* Do we have a password? */    
    CFTypeRef password = CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);
    if (password &&
        (CFGetTypeID(password) == CFStringGetTypeID() && CFStringGetLength((CFStringRef)password)))
        return kCUICredentialReturnCredentialFinished;
    else
        return kCUICredentialNotFinished;
}

