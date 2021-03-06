//
//  PasswordCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPasswordCredentialProvider.h"
#include "CUIPasswordCredential.h"

#include <libkern/OSAtomic.h>

extern "C" {
    void *CUIPasswordCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIPasswordCredentialProvider : public CUIProvider {
    
private:
    int32_t _retainCount;
    CUIControllerRef _controller;

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
            CFEqual(interfaceID, IUnknownUUID)) {
            *ppv = this;
            AddRef();
            CFRelease(interfaceID);
            return S_OK;
        }
        CFRelease(interfaceID);
        return E_NOINTERFACE;
    }
  
    CFArrayRef copyMatchingCredentials(CFDictionaryRef attributes,
                                       CUIUsageFlags usageFlags,
                                       CFIndex *defaultCredentialIndex,
                                       CFErrorRef *error) {
        CUIPasswordCredential *passwordCred = new CUIPasswordCredential();
        CUICredentialRef credRef;
        CFArrayRef creds;
        
        if (!passwordCred->initWithControllerAndAttributes(_controller, usageFlags, attributes, error)) {
            passwordCred->Release();
            return NULL;
        }
        
        credRef = CUICredentialCreate(CFGetAllocator(_controller), passwordCred);
        if (credRef == NULL) {
            passwordCred->Release();
            return NULL;
        }
        
        creds = CFArrayCreate(CFGetAllocator(_controller), (const void **)&credRef, 1, &kCFTypeArrayCallBacks);
        
        CFRelease(credRef);
        passwordCred->Release();
        
        return creds;
    }

    CFSetRef getWhitelistedAttributeKeys(void) {
        return NULL;
    }
    
    Boolean initWithController(CUIControllerRef controller, CFErrorRef *error) {
        if (CUIControllerGetUsageFlags(controller) & kCUIUsageFlagsRequireCertificates)
            return false;

        _controller = (CUIControllerRef)CFRetain(controller);
        return true;
    }
    
    CUIPasswordCredentialProvider() {
        CFPlugInAddInstanceForFactory(kPasswordCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
    }

protected:
    
    ~CUIPasswordCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kPasswordCredentialProviderFactoryID);
    }
    
private:
    
};

__attribute__((visibility("default"))) void *
CUIPasswordCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIPasswordCredentialProvider;
    
    return NULL;
}
