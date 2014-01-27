//
//  SampleCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "SampleCredentialProvider.h"
#include "SampleCredential.h"

#include <libkern/OSAtomic.h>

extern "C" {
    void *SampleCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class SampleCredentialProvider : public CUIProvider {
    
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
   
    /*
     * This method returns an array of credentials enumerated by this provider. We only
     * return one here.
     */ 
    CFArrayRef copyMatchingCredentials(CFDictionaryRef attributes,
                                       CUIUsageFlags usageFlags,
                                       CFIndex *defaultCredentialIndex,
                                       CFErrorRef *error) {
        SampleCredential *passwordCred = new SampleCredential();
        CUICredentialRef credRef;
        CFArrayRef creds;
     
        /* Initialize a new credential */   
        if (!passwordCred->initWithControllerAndAttributes(_controller, usageFlags, attributes, error)) {
            passwordCred->Release();
            return NULL;
        }
       
        /* Wrap it up as a CUICredential and return it in an array */ 
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
    
    SampleCredentialProvider() {
        CFPlugInAddInstanceForFactory(kSampleCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
    }
    
protected:
    ~SampleCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kSampleCredentialProviderFactoryID);
    }
    
private:
    
};

__attribute__((visibility("default"))) void *
SampleCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new SampleCredentialProvider;
    
    return NULL;
}
