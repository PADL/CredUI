//
//  PasswordCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUICertificateCredentialProvider.h"
#include "CUICertificateCredential.h"

extern "C" {
    void *CUICertificateCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

CFMutableDictionaryRef
CUICertificateCredentialProvider::createCertificateAttributesFromCUIAttributes(CFDictionaryRef attributes,
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
    
    CFDictionarySetValue(keychainAttrs, kSecClass, kSecClassIdentity);
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
CUICertificateCredentialProvider::copyMatchingIdentities(CFDictionaryRef attributes, CFTypeRef targetName, CFErrorRef *error)
{
    CFMutableDictionaryRef query = NULL;
    CFArrayRef result = NULL;
    
    query = createCertificateAttributesFromCUIAttributes(attributes, targetName);
    if (query == NULL)
        return NULL;
    
    CFDictionarySetValue(query, kSecReturnRef,        kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchLimit,       kSecMatchLimitAll);
    
    if (SecItemCopyMatching(query, (CFTypeRef *)&result) != errSecSuccess) {
        if (targetName) {
            CFDictionaryRemoveValue(query, kSecAttrService);
            SecItemCopyMatching(query, (CFTypeRef *)&result);
        }
    }
    
    CFRelease(query);
    
    return result;
}

CFArrayRef
CUICertificateCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFArrayRef identities;
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
   
    if (creds == NULL)
        return NULL;
 
    identities = copyMatchingIdentities(attributes, targetName, error);
    if (identities) {
        for (CFIndex index = 0; index < CFArrayGetCount(identities); index++) {
            SecIdentityRef identity = (SecIdentityRef)CFArrayGetValueAtIndex(identities, index);
            CUICertificateCredential *identityCred;
            CUICredentialRef credRef;

            identityCred = new CUICertificateCredential();
            if (!identityCred->initWithSecIdentity(identity, _usageFlags, NULL)) {
                identityCred->Release();
                continue;
            }
            
            credRef = CUICredentialCreate(CFGetAllocator(_controller), identityCred);
            if (credRef == NULL) {
                identityCred->Release();
                continue;
            }
            
            CFArrayAppendValue(creds, credRef);
            
            CFRelease(credRef);
            identityCred->Release();
        }

        CFRelease(identities);
    }
    
    return creds;
}

__attribute__((visibility("default"))) void *
CUICertificateCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUICertificateCredentialProvider;
    
    return NULL;
}
