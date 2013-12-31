//
//  PersonaCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CPersonaCredentialProvider.h"
#include "CPersonaCredential.h"

#include <libkern/OSAtomic.h>

// 58733A29-A6A5-4E57-93EB-200D9411F686
#define kPersonaCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x58, 0x73, 0x3A, 0x29, 0xA6, 0xA5, 0x4E, 0x57, 0x93, 0xEB, 0x20, 0x0D, 0x94, 0x11, 0xF6, 0x86)

const CFStringRef kGSSAttrBrowserIDAssertion = CFSTR("kGSSAttrBrowserIDAssertion");
const CFStringRef kGSSAttrBrowserIDIdentity  = CFSTR("kGSSAttrBrowserIDIdentity");

extern "C" {
    void *CPersonaCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CPersonaCredentialProvider : public CUIProvider {
    
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
    
    CUICredentialContext *createCredentialWithAttributes(CFDictionaryRef attributes, CFErrorRef *error) {
        CPersonaCredential *passwordCred = new CPersonaCredential();
        
        if (!passwordCred->initWithControllerAndAttributes(_controller, attributes, error)) {
            passwordCred->Release();
            return NULL;
        }
        
        return passwordCred;
    }
    
    Boolean initWithController(CUIControllerRef controller, CFErrorRef *error) {
        _controller = (CUIControllerRef)CFRetain(controller);
        return true;
    }

    CFArrayRef createOtherCredentials(CFErrorRef *error) {
        return NULL;
    }
    
    CPersonaCredentialProvider() {
        CFPlugInAddInstanceForFactory(kPersonaCredentialProviderFactoryID);
        _retainCount = 1;
    }
    
protected:
    
    ~CPersonaCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kPersonaCredentialProviderFactoryID);
    }
    
private:
    
};

void *
CPersonaCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CPersonaCredentialProvider;
    
    return NULL;
}
