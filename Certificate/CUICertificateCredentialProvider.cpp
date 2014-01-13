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

CUICredentialRef
CUICertificateCredentialProvider::createCredentialWithIdentity(SecIdentityRef identity)
{
    CUICertificateCredential *identityCred;
    CUICredentialRef credRef;
    
    identityCred = new CUICertificateCredential();
    if (!identityCred->initWithSecIdentity(identity, _usageFlags, NULL)) {
        identityCred->Release();
        return NULL;
    }
    
    credRef = CUICredentialCreate(CFGetAllocator(_controller), identityCred);
    if (credRef == NULL) {
        identityCred->Release();
        return NULL;
    }
    
    identityCred->Release();

    return credRef;
}

CFArrayRef
CUICertificateCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
                                                          CFIndex *defaultCredentialIndex,
                                                          CFErrorRef *error)
{
    CFArrayRef identities;
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
    SecCertificateRef certificate = NULL;
    SecIdentityRef identity = NULL;
   
    if (creds == NULL)
        return NULL;
 
    if (attributes) {
        identity = (SecIdentityRef)CFDictionaryGetValue(attributes, kCUIAttrCredentialSecIdentity);
        if (identity) {
            CFRetain(identity);
        } else {
            certificate = (SecCertificateRef)CFDictionaryGetValue(attributes, kCUIAttrCredentialSecCertificate);
            if (certificate)
                (void)SecIdentityCreateWithCertificate(NULL, certificate, &identity);
        }
    }

    if (identity) {
        CUICredentialRef credRef = createCredentialWithIdentity(identity);
        
        if (credRef) {
            CFArrayAppendValue(creds, credRef);
            CFRelease(credRef);
        }
        
        CFRelease(identity);
    } else if (_usageScenario == kCUIUsageScenarioNetwork) {
        identities = copyMatchingIdentities(attributes, targetName, error);
        if (identities) {
            for (CFIndex index = 0; index < CFArrayGetCount(identities); index++) {
                CUICredentialRef credRef;
                
                credRef = createCredentialWithIdentity((SecIdentityRef)CFArrayGetValueAtIndex(identities, index));
                if (credRef == NULL)
                    continue;

                CFArrayAppendValue(creds, credRef);
                CFRelease(credRef);
            }

            CFRelease(identities);
        }
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
