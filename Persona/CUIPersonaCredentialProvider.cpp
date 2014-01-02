//
//  PersonaCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPersonaCredentialProvider.h"
#include "CUIPersonaCredential.h"

#include <libkern/OSAtomic.h>

// 58733A29-A6A5-4E57-93EB-200D9411F686
#define kPersonaCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x58, 0x73, 0x3A, 0x29, 0xA6, 0xA5, 0x4E, 0x57, 0x93, 0xEB, 0x20, 0x0D, 0x94, 0x11, 0xF6, 0x86)

const CFStringRef kCUIAttrCredentialBrowserIDAssertion = CFSTR("kCUIAttrCredentialBrowserIDAssertion");
const CFStringRef kCUIAttrCredentialBrowserIDIdentity  = CFSTR("kCUIAttrCredentialBrowserIDIdentity");

const CFStringRef kCUIAttrClassBrowserID = CFSTR("1.3.6.1.4.1.5322.24.1.17");

extern "C" {
    void *CUIPersonaCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIPersonaCredentialProvider : public CUIProvider {
    
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
        CUIPersonaCredential *personaCred = new CUIPersonaCredential();
       
        if (!personaCred->initWithControllerAndAttributes(_controller, attributes, error)) {
            personaCred->Release();
            return NULL;
        }
        
        return personaCred;
    }
    
    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error) {
        if (usageScenario != kCUIUsageScenarioNetwork)
            return false;
        if (usageFlags & kCUIUsageFlagsDoNotShowUI)
            return false;

        _controller = (CUIControllerRef)CFRetain(controller);
        return true;
    }

    CFArrayRef createOtherCredentials(CFErrorRef *error) {
        return NULL;
    }
    
    CUIPersonaCredentialProvider() {
        CFPlugInAddInstanceForFactory(kPersonaCredentialProviderFactoryID);
        _retainCount = 1;
    }
    
protected:
    
    ~CUIPersonaCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kPersonaCredentialProviderFactoryID);
    }
    
private:
    
};

__attribute__((visibility("default"))) void *
CUIPersonaCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIPersonaCredentialProvider;
    
    return NULL;
}
