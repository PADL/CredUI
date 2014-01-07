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
    CFStringRef defaultUsername = NULL;
    CUIFieldRef fields[4] = { 0 };
    size_t cFields = 0;
    
    if (error != NULL)
        *error = NULL;
    
    _controller = (CUIControllerRef)CFRetain(controller);
    
    if (attributes) {
        if (!CUIShouldEnumerateForPasswordClass(attributes))
            return false;
        
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
            case kCUIAttributeSourceUnknown:
                break;
        }
       
        /*
         * Get the default user name for display.
         */
        defaultUsername = CUIGetDefaultUsername(attributes);
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
   
    /*
     * The "submit button". This is called when the credential is about to be submitted.
     */
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, NULL, NULL,
                                       ^(CUIFieldRef field, CFTypeRef value) {
        if (hasPlaceholderPassword()) {
            CFUUIDRef persistenceFactoryID = (CFUUIDRef)CFDictionaryGetValue(_attributes, kCUIAttrPersistenceFactoryID);

            if (persistenceFactoryID) {
                CUICredentialPersistence *persistence = __CUIControllerCreatePersistenceForFactoryID(_controller, persistenceFactoryID);
                if (persistence) {
                    CFTypeRef password = persistence->extractPassword(_attributes, NULL);
                    if (password) {
                        CFDictionarySetValue(_attributes, kCUIAttrCredentialPassword, password);
                        CFRelease(password);
                    }
                    persistence->Release();
                }
            }
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
   
    if (username == NULL || CFStringGetLength(username) == 0)
        return kCUICredentialNotFinished;

    if (hasPassword())
        return kCUICredentialReturnCredentialFinished;
    else
        return kCUICredentialNotFinished;
}

/*
 * If the user wants this credential to be persisted, do so here.
 */
Boolean CUIPasswordCredential::savePersisted(CFErrorRef *error)
{
    Boolean ret = false;
    CUICredentialPersistence *persistence;
    
    if (error)
        *error = NULL;

    /*
     * We only persist new credentials, that is, credentials that are not
     * created by the keychain os GSSItem providers.
     */
    if (CUIGetAttributeSource(_attributes) != kCUIAttributeSourceUser)
        return true;
    
    persistence = CUICreatePersistenceForSource(_controller,
                                                _generic ? kCUIAttributeSourceKeychain : kCUIAttributeSourceGSSItem);
    if (persistence) {
        ret = persistence->addCredentialWithAttributes(_attributes, error);
        persistence->Release();
    }

    return ret;
}
