//
//  CUIKeychainCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <libkern/OSAtomic.h>

#include <Security/Security.h>

#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"
#include "CUIKeychainCredentialProvider.h"
#include "CUIKeychainCredential.h"

extern "C" {
    void *CUIKeychainCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

static CFNumberRef kCUISecPasswordType;
static CFStringRef kCredUI = CFSTR("CredUI");

static void
_CUIKeychainProviderUtilitiesInit(void) __attribute((__constructor__));

static void
_CUIKeychainProviderUtilitiesInit(void)
{
    uint32_t num;
    
    num = 'CUIP';
    kCUISecPasswordType = CFNumberCreate(NULL, kCFNumberSInt32Type, &num);
}

CFMutableDictionaryRef
CUIKeychainCredentialProvider::createCUIAttributesFromKeychainAttributes(CFDictionaryRef keychainAttrs)
{
    CFMutableDictionaryRef attributes;
    
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL)
        return NULL;
    
    CFTypeRef accountName = CFDictionaryGetValue(keychainAttrs, kSecAttrAccount);
    if (accountName) {
        CFDictionarySetValue(attributes, kCUIAttrName, accountName);
        CFDictionarySetValue(attributes, kCUIAttrNameDisplay, accountName);
    } else {
        CFRelease(attributes);
        return NULL;
    }
    
    CFDictionarySetValue(attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    CFDictionarySetValue(attributes, kCUIAttrCredentialExists, kCFBooleanTrue);
    CFDictionarySetValue(attributes, kCUIAttrSupportGSSCredential, kCFBooleanFalse);
    CFDictionarySetValue(attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername); // XXX
    CFDictionarySetValue(attributes, kCUIAttrCredentialPassword, kCFBooleanTrue); // XXX
    CFDictionarySetValue(attributes, kCUIAttrPersistenceFactoryID, kKeychainCredentialProviderFactoryID);

    CFTypeRef keychainItemRef = CFDictionaryGetValue(keychainAttrs, kSecValueRef);
    if (keychainItemRef)
        CFDictionarySetValue(attributes, kCUIAttrSecKeychainItem, keychainItemRef);
    
    CFTypeRef accessGroup = CFDictionaryGetValue(keychainAttrs, kSecAttrAccessGroup);
    if (accessGroup)
        CFDictionarySetValue(attributes, kSecAttrAccessGroup, accessGroup);
    
    return attributes;
}

CFMutableDictionaryRef
CUIKeychainCredentialProvider::createKeychainAttributesFromCUIAttributes(CFDictionaryRef attributes,
                                                                         CFTypeRef targetName)
{
    CFMutableDictionaryRef keychainAttrs;
    CFStringRef name;
    
    keychainAttrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (keychainAttrs == NULL) {
        return NULL;
    }
    
    if (targetName) {
        CFStringRef targetDisplayName = CUICopyTargetDisplayName(targetName);
        
        if (targetDisplayName) {
            CFDictionarySetValue(keychainAttrs, kSecAttrService, targetDisplayName);
            CFRelease(targetDisplayName);
        }
    }
    
    CFDictionarySetValue(keychainAttrs, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(keychainAttrs, kSecAttrType, kCUISecPasswordType);
    if (attributes) {
        name = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);
        if (name)
            CFDictionarySetValue(keychainAttrs, kSecAttrAccount, name);
    }
    
    CFTypeRef accessGroup = CFDictionaryGetValue(keychainAttrs, kSecAttrAccessGroup);
    if (accessGroup)
        CFDictionarySetValue(keychainAttrs, kSecAttrAccessGroup, accessGroup);
    
    return keychainAttrs;
}

CFArrayRef
CUIKeychainCredentialProvider::copyMatching(CFDictionaryRef attributes, CFTypeRef targetName, CFErrorRef *error)
{
    CFMutableDictionaryRef query = NULL;
    CFArrayRef result = NULL;
    
    query = createKeychainAttributesFromCUIAttributes(attributes, targetName);
    if (query == NULL)
        return NULL;
    
    CFDictionarySetValue(query, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
    
    if (SecItemCopyMatching(query, (CFTypeRef *)&result) != errSecSuccess) {
        if (targetName) {
            CFDictionarySetValue(query, kSecAttrService, kCredUI);
            SecItemCopyMatching(query, (CFTypeRef *)&result);
        }
    }
    
    CFRelease(query);
    
    return result;
}

Boolean
CUIKeychainCredentialProvider::setPasswordAttr(CFMutableDictionaryRef keychainAttrs, CFDictionaryRef attributes)
{
    CFTypeRef password = CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword);
    
    if (password) {
        if (CFGetTypeID(password) == CFStringGetTypeID()) {
            CFDataRef passwordData;
            
            passwordData = CFStringCreateExternalRepresentation(kCFAllocatorDefault,
                                                                (CFStringRef)password, kCFStringEncodingUTF8, 0);
            
            if (passwordData == NULL)
                return false;
            
            CFDictionarySetValue(keychainAttrs, kSecValueData, passwordData);
            CFRelease(passwordData);
        } else if (CFGetTypeID(password) == CFDataGetTypeID()) {
            CFDictionarySetValue(keychainAttrs, kSecValueData, password);
        } else if (CFGetTypeID(password) != CFBooleanGetTypeID()) {
            /* Hmm, what's going on here? */
        }
    }
    
    return true;
}

CFMutableDictionaryRef
CUIKeychainCredentialProvider::createQuery(CFDictionaryRef attributes)
{
    CFMutableDictionaryRef query;
    SecKeychainItemRef item;

    item = (SecKeychainItemRef)CFDictionaryGetValue(attributes, kCUIAttrSecKeychainItem);
    if (item == NULL)
        return NULL;

    query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (query == NULL)
        return NULL;
    
    CFDictionarySetValue(query, kSecValueRef, item);
    
    return query;
}

Boolean
CUIKeychainCredentialProvider::addCredentialWithAttributes(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFMutableDictionaryRef keychainAttrs;
    OSStatus osret;
    CFTypeRef itemRef = NULL;
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    
    if (targetName == NULL)
        targetName = kCredUI;
    
    keychainAttrs = createKeychainAttributesFromCUIAttributes(attributes, targetName);
    if (keychainAttrs == NULL)
        return false;
    
    CFStringRef description = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("CredUI password for %@"),
                                                       CFDictionaryGetValue(attributes, kCUIAttrName));
    
    if (description == NULL) {
        CFRelease(keychainAttrs);
        return false;
    }
    
    CFDictionarySetValue(keychainAttrs, kSecAttrDescription, description);
    CFDictionarySetValue(keychainAttrs, kSecAttrLabel, description);
    CFRelease(description);
    
    if (!setPasswordAttr(keychainAttrs, attributes)) {
        CFRelease(keychainAttrs);
        return false;
    }
    
    osret = SecItemAdd(keychainAttrs, &itemRef);
    
    if (itemRef)
        CFRelease(itemRef);
    
    CFRelease(keychainAttrs);
    
    return !osret;
}

Boolean
CUIKeychainCredentialProvider::updateCredential(CUICredentialRef credential, CFErrorRef *error)
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    Boolean ret = false;
    CFMutableDictionaryRef keychainAttrs = createKeychainAttributesFromCUIAttributes(attributes, NULL);
    CFDictionaryRef query = createQuery(attributes);
    
    if (keychainAttrs && query &&
        setPasswordAttr(keychainAttrs, attributes)) {
        ret = (SecItemUpdate(query, keychainAttrs) == errSecSuccess);
    } else {
        ret = false;
    }
    
    if (query)
        CFRelease(query);
    if (keychainAttrs)
        CFRelease(keychainAttrs);
    
    return ret;
}

Boolean
CUIKeychainCredentialProvider::deleteCredential(CUICredentialRef credential, CFErrorRef *error)
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    Boolean ret;
    CFDictionaryRef query = createQuery(attributes);
    
    if (query == NULL)
        return false;
    
    ret = (SecItemDelete(query) == errSecSuccess);
    
    CFRelease(query);
    
    return ret;
}

CFTypeRef
CUIKeychainCredentialProvider::extractPassword(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFTypeRef password = NULL;
    CFMutableDictionaryRef query;
    CFDataRef result = NULL;
     
    if (error)
        *error = NULL;
 
    query = createQuery(attributes);
    if (query) {
        CFDictionaryAddValue(query, kSecReturnData, kCFBooleanTrue);
        
        SecItemCopyMatching(query, (CFTypeRef *)&result);
    
        if (result) {
            password = CFStringCreateFromExternalRepresentation(CFGetAllocator(attributes), result, kCFStringEncodingUTF8);
            CFRelease(result);
        }
        CFRelease(query);
    }

    return password;
}

CFArrayRef
CUIKeychainCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
                                                       CFIndex *defaultCredentialIndex,
                                                       CFErrorRef *error)
{
    CFArrayRef items;
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    __block CFIndex cCreds = 0;

    items = copyMatching(attributes, targetName, error);
    if (items) {
        for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
            CFDictionaryRef keychainAttrs = (CFDictionaryRef)CFArrayGetValueAtIndex(items, index);
            CFDictionaryRef attrs = createCUIAttributesFromKeychainAttributes(keychainAttrs);
            
            if (attrs == NULL)
                continue;
            
            _CUIControllerEnumerateCredentialsWithFlags(_controller,
                                                        kCUIUsageFlagsExcludePersistedCreds,
                                                        attrs,
                                                        ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
                 CUIKeychainCredential *itemCred;
                 CUICredentialRef credRef;
                 
                 if (cred == NULL)
                     return;

                 if (isDefault)
                     *defaultCredentialIndex = cCreds;
                 
                 itemCred = new CUIKeychainCredential();
                 if (!itemCred->initWithCredential(cred, this)) {
                     itemCred->Release();
                     return;
                 }
                 
                 credRef = CUICredentialCreate(CFGetAllocator(_controller), itemCred);
                 if (credRef == NULL) {
                     itemCred->Release();
                     return;
                 }
                 
                 CFArrayAppendValue(creds, credRef);
                 cCreds++;
                 
                 CFRelease(credRef);
                 itemCred->Release();
             });
            
            CFRelease(attrs);
        }
        
        CFRelease(items);
    }
    
    return creds;
}

Boolean CUIKeychainCredentialProvider::initWithController(CUIControllerRef controller,
                                                          CUIUsageScenario usageScenario,
                                                          CUIUsageFlags usageFlags,
                                                          CFErrorRef *error)
{
    if (usageScenario != kCUIUsageScenarioNetwork)
        return false;

    if ((usageFlags & kCUIUsageFlagsGeneric) == 0 ||
        (usageFlags & (kCUIUsageFlagsRequireCertificates | kCUIUsageFlagsExcludePersistedCreds)))
        return false;
    
    _controller = (CUIControllerRef)CFRetain(controller);
    _usageScenario = usageScenario;
    _usageFlags = usageFlags;
    
    return true;
}


__attribute__((visibility("default"))) void *
CUIKeychainCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIKeychainCredentialProvider;
    
    return NULL;
}
