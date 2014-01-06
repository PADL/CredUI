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

// 5885FAD6-4C4B-4A9F-A00A-EE1577571D96
#define kSampleCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x58, 0x85, 0xFA, 0xD6, 0x4C, 0x4B, 0x4A, 0x9F, 0xA0, 0x0A, 0xEE, 0x15, 0x77, 0x57, 0x1D, 0x96)

extern "C" {
    void *SampleCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class SampleCredentialProvider : public CUIProvider {
    
private:
    int32_t _retainCount;
    CUIControllerRef _controller;
    CUIUsageScenario _usageScenario;
    CUIUsageFlags _usageFlags;
    
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
                                       CFErrorRef *error) {
        SampleCredential *passwordCred = new SampleCredential();
        CUICredentialRef credRef;
        CFArrayRef creds;
        
        if (!passwordCred->initWithControllerAndAttributes(_controller, _usageFlags, attributes, error)) {
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
    
    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error) {
        _controller = (CUIControllerRef)CFRetain(controller);
        _usageScenario = usageScenario;
        _usageFlags = usageFlags;
        return true;
    }
    
    SampleCredentialProvider() {
        CFPlugInAddInstanceForFactory(kSampleCredentialProviderFactoryID);
        _retainCount = 1;
        _usageScenario = kCUIUsageScenarioInvalid;
        _usageFlags = 0;
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
