//
//  PersonaCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPersonaCredentialProvider.h"
#include "CUIPersonaCredential.h"
#include "CUIProviderUtilities.h"

#include <libkern/OSAtomic.h>

const CFStringRef kCUIAttrCredentialBrowserIDAssertion = CFSTR("kCUIAttrCredentialBrowserIDAssertion");
const CFStringRef kCUIAttrCredentialBrowserIDIdentity  = CFSTR("kCUIAttrCredentialBrowserIDIdentity");
const CFStringRef kCUIAttrCredentialBrowserIDFlags     = CFSTR("kCUIAttrCredentialBrowserIDFlags");

const CFStringRef kCUIAttrClassBrowserID               = CFSTR("1.3.6.1.4.1.5322.24.1.17");

extern "C" {
    void *CUIPersonaCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIPersonaCredentialProvider : public CUIProvider {
    
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
                                       CFIndex *defaultCredentialIndex,
                                       CFErrorRef *error) {
        CUIPersonaCredential *personaCred = new CUIPersonaCredential();
        CUICredentialRef credRef;
        CFArrayRef creds;

        /*
         * Although BrowserID uses certificates, we're going to leave that for providers
         * that actually deal with X.509 certificates.
         */
        if (_usageFlags & kCUIUsageFlagsRequireCertificates)
            return NULL;

        if (!personaCred->initWithControllerAndAttributes(_controller, _usageFlags, attributes, error)) {
            personaCred->Release();
            return NULL;
        }
        
        credRef = CUICredentialCreate(CFGetAllocator(_controller), personaCred);
        if (credRef == NULL) {
            personaCred->Release();
            return NULL;
        }
        
        creds = CFArrayCreate(CFGetAllocator(_controller), (const void **)&credRef, 1, &kCFTypeArrayCallBacks);
        
        CFRelease(credRef);
        personaCred->Release();

        return creds;
    }
    
    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error) {
        if (usageScenario != kCUIUsageScenarioNetwork)
            return false;

        if (usageFlags & (kCUIUsageFlagsGeneric | kCUIUsageFlagsDoNotShowUI))
            return false;
        
        _controller = (CUIControllerRef)CFRetain(controller);
        _usageScenario = usageScenario;
        _usageFlags = usageFlags;
        
        return true;
    }

    CUIPersonaCredentialProvider() {
        CFPlugInAddInstanceForFactory(kPersonaCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
        _usageScenario = kCUIUsageScenarioInvalid;
        _usageFlags = 0;
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
