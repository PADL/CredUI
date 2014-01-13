//
//  CUILoginIdentityCredentialProvider.h
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUILoginIdentityCredentialProvider__
#define __CredUI__CUILoginIdentityCredentialProvider__

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CUIProvider.h>

// 37690CFE-2BEE-4046-9CD5-D632AB97AFBB
#define kLoginIdentityCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x37, 0x69, 0x0C, 0xFE, 0x2B, 0xEE, 0x40, 0x46, 0x9C, 0xD5, 0xD6, 0x32, 0xAB, 0x97, 0xAF, 0xBB)

class CUILoginIdentityCredentialProvider : public CUIProvider {
    
private:
    int32_t _retainCount;
    CUIControllerRef _controller;
    CUIUsageFlags _usageFlags;
    CSIdentityAuthorityRef _authority;
    
public:
    ULONG AddRef(void) {
        return OSAtomicIncrement32Barrier(&_retainCount);
    }
    
    ULONG Release(void) {
        int32_t retainCount = OSAtomicDecrement32Barrier(&_retainCount);
        
        if (retainCount <= 0) {
            delete this;
            return 0;
        }
        return retainCount;
    }
    
    HRESULT QueryInterface(REFIID riid, void **ppv) {
        CFUUIDRef interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, riid);
        
        if (CFEqual(interfaceID, kCUIProviderInterfaceID) ||
            CFEqual(interfaceID, IUnknownUUID))
            *ppv = (CUIProvider *)this;
        else
            *ppv = NULL;
        
        if (*ppv != NULL)
            AddRef();
        
        CFRelease(interfaceID);
        
        return *ppv ? S_OK : E_NOINTERFACE;
    }

    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error);
    
    CFArrayRef copyMatchingCredentials(CFDictionaryRef attributes, CFIndex *defaultCredentialIndex, CFErrorRef *error);

    // helpers    
    CSIdentityQueryRef createQuery(CFDictionaryRef attributes);
    CFDictionaryRef copyAttributesForIdentity(CSIdentityRef identity);

    CUILoginIdentityCredentialProvider() {
        CFPlugInAddInstanceForFactory(kLoginIdentityCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
        _usageFlags = 0;
        _authority = NULL;
    }
    
protected:
    
    ~CUILoginIdentityCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        if (_authority)
            CFRelease(_authority);
        CFPlugInRemoveInstanceForFactory(kLoginIdentityCredentialProviderFactoryID);
    }
    
};

#endif /* defined(__CredUI__CUILoginIdentityCredentialProvider__) */
