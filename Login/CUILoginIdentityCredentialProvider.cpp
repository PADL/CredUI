//
//  CUILoginIdentityCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CUILoginIdentityCredentialProvider.h"
#include "CUILoginIdentityCredential.h"

extern "C" {
    void *CUILoginIdentityCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
}

CSIdentityQueryRef
CUILoginIdentityCredentialProvider::createQuery(CFDictionaryRef attributes)
{
    CSIdentityQueryRef query;
    CFStringRef name = NULL;

    if (attributes)
        name = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);

    if (name)
        query = CSIdentityQueryCreateForName(kCFAllocatorDefault, name, kCSIdentityQueryStringEquals, kCSIdentityClassUser, _authority);
    else
        query = CSIdentityQueryCreate(kCFAllocatorDefault, kCSIdentityClassUser, _authority);
    if (query == NULL)
        return NULL;

    return query;
}

CFDictionaryRef
CUILoginIdentityCredentialProvider::copyAttributesForIdentity(CSIdentityRef identity)
{
    CFMutableDictionaryRef attrs;

    attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attrs == NULL)
        return NULL;

    if (!CSIdentityIsEnabled(identity))
        return NULL;

    CFDictionarySetValue(attrs, kCUIAttrCSIdentity,           identity);
    CFDictionarySetValue(attrs, kCUIAttrUUID,                 CSIdentityGetUUID(identity));
    CFDictionarySetValue(attrs, kCUIAttrName,                 CSIdentityGetPosixName(identity));
    CFDictionarySetValue(attrs, kCUIAttrNameDisplay,          CSIdentityGetFullName(identity));
    CFDictionarySetValue(attrs, kCUIAttrIdentityFactoryID,    kLoginIdentityCredentialProviderFactoryID);

    SecCertificateRef certificate = CSIdentityGetCertificate(identity);
    if (certificate)
        CFDictionarySetValue(attrs, kCUIAttrCredentialSecCertificate, certificate);

    CFDataRef imageData = CSIdentityGetImageData(identity);
    if (imageData) {
        CFDictionarySetValue(attrs, kCUIAttrImageData, imageData);
        CFDictionarySetValue(attrs, kCUIAttrImageDataType, CSIdentityGetImageDataType(identity));
    }
        
    return attrs;
}

CFArrayRef
CUILoginIdentityCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
                                                            CFIndex *defaultCredentialIndex,
                                                            CFErrorRef *error)
{
    CSIdentityQueryRef query;
    CFArrayRef items;
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
    __block CFIndex cCreds = 0;

    query = createQuery(attributes);
    if (query == NULL)
        return NULL;

    if (!CSIdentityQueryExecute(query, 0, error)) {
        CFRelease(query);
        return NULL;
    }

    items = CSIdentityQueryCopyResults(query);
    if (items) {
        for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
            CSIdentityRef identity = (CSIdentityRef)CFArrayGetValueAtIndex(items, index);
            CFDictionaryRef attrs = copyAttributesForIdentity(identity);

            if (attrs == NULL)
                continue;

            _CUIControllerEnumerateCredentialsWithFlags(_controller,
                                                        kCUIUsageFlagsExcludeIdentityCreds,
                                                        attrs,
                                                        ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
                 CUILoginIdentityCredential *itemCred;
                 CUICredentialRef credRef;
                 
                 if (cred == NULL)
                     return;

                 if (isDefault)
                     *defaultCredentialIndex = cCreds;
                 
                 itemCred = new CUILoginIdentityCredential;
                 if (!itemCred->initWithCredential(cred)) {
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

Boolean CUILoginIdentityCredentialProvider::initWithController(CUIControllerRef controller,
                                                               CUIUsageScenario usageScenario,
                                                               CUIUsageFlags usageFlags,
                                                               CFErrorRef *error)
{
    if (usageScenario != kCUIUsageScenarioLogin)
        return false;

    _authority = CSGetLocalIdentityAuthority();
    if (_authority == NULL)
        return false;

    _controller = (CUIControllerRef)CFRetain(controller);
    _usageFlags = usageFlags;
    
    return true;
}


__attribute__((visibility("default"))) void *
CUILoginIdentityCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUILoginIdentityCredentialProvider;
    
    return NULL;
}
