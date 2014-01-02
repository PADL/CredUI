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
        switch (CUIGetAttributeSource(attributes)) {
            case kCUIAttributeSourceGSSItem:
                /*
                 * Ignore GSS items if we are requesting generic credentials (i.e. the
                 * caller expects to get a pasword back) or if we don't have a password set.
                 */
                if ((usageFlags & kCUIUsageFlagsGeneric) ||
                    !CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
                return false;
                break;
            case kCUIAttributeSourceKeychain:
                if ((usageFlags & kCUIUsageFlagsGeneric) == 0)
                    return false;
                break;
            case kCUIAttributeSourceUser:
                break;
        }
        
        if (CUIGetAttributeSource(attributes) != kCUIAttributeSourceUser &&
            !CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword))
            return false;

        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = CFDictionaryGetValue(attributes, kCUIAttrName);
        
        _attributes = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, attributes);
    } else {
        _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    }
    
    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, CFSTR("PasswordCredentialProvider"));
    if (usageFlags & kCUIUsageFlagsGeneric) {
        _generic = true;
        CFDictionarySetValue(_attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrSupportGSSCredential, kCFBooleanTrue);
    }
 
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Password Credential"), NULL);
    
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

void CUIPasswordCredential::didSubmit(void)
{
    if (isPlaceholderPassword()) {
        // now we need to get the real password if it's a keychain item
        switch (CUIGetAttributeSource(_attributes)) {
            case kCUIAttributeSourceGSSItem:
            case kCUIAttributeSourceUser:
                break;
            case kCUIAttributeSourceKeychain:
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
    }
}

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
