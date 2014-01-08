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

CFArrayRef
CUICertificateCredentialProvider::copyMatchingIdentities(CFDictionaryRef attributes, CFTypeRef targetName, CFErrorRef *error)
{
    CFMutableDictionaryRef query = NULL;
    CFArrayRef result = NULL;
    CFStringRef name;
    
    query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                      &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (query == NULL)
        return NULL;

    if (attributes &&
        (name = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName))) {
        CFDictionarySetValue(query, kSecMatchSubjectContains, name);
    }

    CFDictionarySetValue(query, kSecClass,      kSecClassIdentity);
    CFDictionarySetValue(query, kSecReturnRef,  kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);

    SecItemCopyMatching(query, (CFTypeRef *)&result);
    
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
