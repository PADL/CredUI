//
//  CUIPasswordCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <Security/Security.h>

#include "CUIPasswordCredentialProvider.h"
#include "CUIPasswordCredential.h"
#include "CUIProviderUtilities.h"

static CFStringRef
kPasswordCredentialProvider = CFSTR("com.padl.CredUI.Providers.PasswordCredentialProvider");

Boolean
CUIPasswordCredential::initWithControllerAndAttributes(CUIControllerRef controller,
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
        /*
         * Ignore persisted credentials without a password, they're no use to us.
         */
        if (CUIIsPersistedCredential(attributes) && !hasPassword(attributes))
            return false;
        /*
         * For the login scenario, specific users must always be enumerated by the provider.
         * This is so that a user's image and other metadata is available to CredUI.
         */
        else if (isLoginUsageScenario() && !CUIIsIdentityCredential(attributes))
            return false;
        
        /*
         * Get the default user name for display.
         */
        defaultUsername = CUICopyDefaultUsername(attributes);
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
   
    /*
     * Set some default attributes for this credential.
     */ 
    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, kPasswordCredentialProvider);
    CFDictionarySetValue(_attributes, kCUIAttrProviderFactoryID, kPasswordCredentialProviderFactoryID);

    if (usageFlags & kCUIUsageFlagsGeneric) {
        CFDictionarySetValue(_attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrSupportGSSCredential, kCFBooleanTrue);
    }

#if 0
    /*
     * This is the title field that identifies the provider.
     */ 
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Password Credential"), NULL);
#endif
 
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
                                               updateCredentialStatus();
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
                                           updateCredentialStatus();
                                       });
   
    /*
     * The "submit button". This is called when the credential is about to be submitted.
     */
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, NULL, NULL,
                                       ^(CUIFieldRef field, CFTypeRef value) {
        if (hasPlaceholderPassword()) {
            CFUUIDRef persistenceFactoryID = (CFUUIDRef)CFDictionaryGetValue(_attributes, kCUIAttrPersistenceFactoryID);

            if (persistenceFactoryID) {
                CUICredentialPersistence *persistence = CUIControllerCreatePersistenceForFactoryID(_controller, persistenceFactoryID);
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

    if (defaultUsername)
        CFRelease(defaultUsername);
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, cFields, &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;
    
    return true;
}

/*
 * Determine whether the credential is able to be submitted.
 */
void 
CUIPasswordCredential::updateCredentialStatus(void)
{
    CFTypeRef status;
    CFStringRef username = (CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrName);

    if (username == NULL || CFStringGetLength(username) == 0)
        status = kCUICredentialNotFinished;
    else if (hasPassword())
        status = kCUICredentialReturnCredentialFinished;
    else
        status = kCUICredentialNotFinished;

    CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, status);
}

/*
 * If the user wants this credential to be persisted, do so here.
 */
void
CUIPasswordCredential::savePersisted(void (^completionHandler)(CFErrorRef))
{
    CUICredentialPersistence *persistence;
    CFTypeRef attrClass;
    CFBundleRef bundle;
    CFStringRef factorySelector;
    CFTypeRef factoryIDString;
    
    /*
     * Login credentials cannot be saved (XXX this should be enforced at a higher layer).
     *
     * Persisted credentials are updated by the persistence credential provider, we do not
     * need to do anything here.
     */
    assert(!isLoginUsageScenario());
    
    if (CUIIsPersistedCredential(_attributes)) {
        completionHandler(NULL);
        return;
    }

    attrClass = CFDictionaryGetValue(_attributes, kCUIAttrClass);
    if (attrClass == NULL) {
        completionHandler(NULL); // XXX
        return;
    }
    
    bundle = CFBundleGetBundleWithIdentifier(kPasswordCredentialProvider);
    if (CFEqual(attrClass, kCUIAttrClassGeneric))
        factorySelector = CFSTR("CUIGenericPersistenceProviderFactory");
    else
        factorySelector = CFSTR("CUIPersistenceProviderFactory");
    assert(factorySelector);

    factoryIDString = CFBundleGetValueForInfoDictionaryKey(bundle, factorySelector);
    if (factoryIDString && CFGetTypeID(factoryIDString) == CFStringGetTypeID()) {
        CFUUIDRef factoryID = CFUUIDCreateFromString(kCFAllocatorDefault, (CFStringRef)factoryIDString);
        if (factoryID) {
            persistence = CUIControllerCreatePersistenceForFactoryID(_controller, factoryID);
            if (persistence) {
                persistence->addCredentialWithAttributes(_attributes, completionHandler);
                persistence->Release();
            }
            CFRelease(factoryID);
        }
    }
}
