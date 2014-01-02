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

// BFA3619B-1A12-4DBA-801F-33B0874DD76F
#define kPasswordCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xBF, 0xA3, 0x61, 0x9B, 0x1A, 0x12, 0x4D, 0xBA, 0x80, 0x1F, 0x33, 0xB0, 0x87, 0x4D, 0xD7, 0x6F)


extern "C" {
    void *CUIPasswordCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIPasswordCredentialProvider : public CUIProvider {
    
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
        CUIPasswordCredential *passwordCred = new CUIPasswordCredential();
        const CUICredentialContext *contexts[] = { passwordCred };
        
        if (!passwordCred->initWithAttributes(attributes, _usageFlags, error)) {
            passwordCred->Release();
            return NULL;
        }
        
        return CUICredentialContextArrayCreate(CFGetAllocator(_controller), contexts, 1);
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
    
    CUIPasswordCredentialProvider() {
        CFPlugInAddInstanceForFactory(kPasswordCredentialProviderFactoryID);
        _retainCount = 1;
        _usageScenario = kCUIUsageScenarioInvalid;
        _usageFlags = 0;
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
