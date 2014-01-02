//
//  CUIPasswordCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <Security/Security.h>

#include "CUIPasswordCredential.h"
#include "CUIProviderUtilities.h"

Boolean CUIPasswordCredential::initWithControllerAndAttributes(CUIControllerRef controller,
                                                               CUIUsageFlags usageFlags,
                                                               CFDictionaryRef attributes,
                                                               CFErrorRef *error)
{
    CFTypeRef defaultUsername = NULL;
    CUIFieldRef fields[3] = { 0 };
    size_t cFields = 0;
    CFTypeRef targetName;
    
    if (error != NULL)
        *error = NULL;
    
    targetName = CUIControllerGetTargetName(controller);
    if (targetName)
        _targetName = CFRetain(targetName);
    
    if (attributes) {
        /*
         * If the attributes came from GSSItem/keychain, then don't show them unless
         * they have an associated password and the caller wants that source.
         */
        switch (CUIGetAttributeSource(attributes)) {
            case kCUIAttributeSourceGSSItem:
                if ((usageFlags & kCUIUsageFlagsGeneric) ||
                    !CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
                return false;
                break;
            case kCUIAttributeSourceKeychain:
                if ((usageFlags & kCUIUsageFlagsGeneric) == 0 ||
                    !CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
                    return false;
                break;
            case kCUIAttributeSourceUser:
                break;
        }
       
        /*
         * Get the default user name for display.
         */ 
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = CFDictionaryGetValue(attributes, kCUIAttrName);
        
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
   
    /*
     * Set some default attributes for this credential.
     */ 
    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, CFSTR("PasswordCredentialProvider"));
    if (usageFlags & kCUIUsageFlagsGeneric) {
        _generic = true;
        CFDictionarySetValue(_attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrSupportGSSCredential, kCFBooleanTrue);
    }

    /*
     * This is the title field that identifies the provider.
     */ 
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Password Credential"), NULL);
 
    /*
     * The username field.
     */ 
    if ((usageFlags & kCUIUsageFlagsPasswordOnlyOK) == 0) {
        // kCUIUsageFlagsPasswordOnlyOK means do not allow a username to be entered
        // kCUIUsageFlagsKeepUsername means username is read-only
        CUIFieldClass fieldClass = (usageFlags & kCUIUsageFlagsKeepUsername) ? kCUIFieldClassSmallText : kCUIFieldClassEditText;
        
        fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, fieldClass, CFSTR("Username"), defaultUsername,
                                           ^(CUIFieldRef field, CFTypeRef value) {
                                               CFDictionarySetValue(_attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername);
                                               if (value) {
                                                   CFDictionarySetValue(_attributes, kCUIAttrName, value);
                                               } else {
                                                   CFDictionaryRemoveValue(_attributes, kCUIAttrName);
                                               }
                                           });
    }
   
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
   
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, cFields, &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    return true;
}

/*
 * Determine whether the credential is able to be submitted.
 */
const CFStringRef CUIPasswordCredential::getCredentialStatus(void)
{
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrName);
    CFTypeRef password = CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);
   
    if (username == NULL || CFStringGetLength(username) == 0)
        return kCUICredentialNotFinished;

    if (isPlaceholderPassword() || CFStringGetLength((CFStringRef)password))
         return kCUICredentialReturnCredentialFinished;
    else
        return kCUICredentialNotFinished;
}

/*
 * If we have a placeholder password, then get the actual password.
 */
void CUIPasswordCredential::didSubmit(void)
{
    if (isPlaceholderPassword()) {
        switch (CUIGetAttributeSource(_attributes)) {
            case kCUIAttributeSourceKeychain: {
                SecKeychainItemRef itemRef = (SecKeychainItemRef)CFDictionaryGetValue(_attributes, kCUIAttrSecKeychainItemRef);
                
                if (itemRef) {
                    CFMutableDictionaryRef query;
                    CFDataRef result = NULL;
                    
                    query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                    if (query) {
                        CFDictionarySetValue(query, kSecValueRef, itemRef);
                        CFDictionaryAddValue(query, kSecReturnData, kCFBooleanTrue);
                        
                        SecItemCopyMatching(query, (CFTypeRef *)&result);
                        
                        if (result) {
                            CFStringRef password = CFStringCreateFromExternalRepresentation(CFGetAllocator(_attributes), result, kCFStringEncodingUTF8);
                            if (password) {
                                CFDictionarySetValue(_attributes, kCUIAttrCredentialPassword, password);
                                CFRelease(password);
                            }
                            CFRelease(result);
                        }
                        CFRelease(query);
                    }
                }
                break;
            }
            case kCUIAttributeSourceGSSItem:
                // We *could* lookup the GSS item password in the keychain, but that'd be an abstraction violation
                break;
            case kCUIAttributeSourceUser:
            default:
                break;
        }
    }
}

/*
 * If the user wants this credential to be persisted, do so here.
 */
Boolean CUIPasswordCredential::didConfirm(CFErrorRef *error)
{
    Boolean ret = false;

    if (error)
        *error = NULL;

    if (_generic) {
        ret = CUIKeychainStore(_attributes, _targetName, error);
    } else {
        ret = CUIGSSItemAddOrUpdate(_attributes, false, error);
    }

    return ret;
}
