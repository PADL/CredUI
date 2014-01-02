//
//  CUIKeychainCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CredUICore_Private.h>

#include <Security/Security.h>

#include "CUIProviderUtilities.h"

#include "CUIKeychainCredentialProvider.h"
#include "CUIKeychainCredential.h"

// 43022342-F1D5-424D-9D99-2973762F046D
#define kKeychainCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x43, 0x02, 0x23, 0x42, 0xF1, 0xD5, 0x42, 0x4D, 0x9D, 0x99, 0x29, 0x73, 0x76, 0x2F, 0x04, 0x6D)

extern "C" {
    void *CUIKeychainCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIKeychainCredentialProvider : public CUIProvider {
    
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
        CFArrayRef items;
        CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                       0,
                                                       &kCUICredentialContextArrayCallBacks);
        CFTypeRef targetName = CUIControllerGetTargetName(_controller);
        
        items = CUIKeychainCopyMatching(attributes, targetName, error);
        if (items) {
            for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
                CFDictionaryRef keychainAttrs = (CFDictionaryRef)CFArrayGetValueAtIndex(items, index);
                CFDictionaryRef attrs = CUICreateCUIAttributesFromKeychainAttributes(keychainAttrs, true);
                
                if (attrs == NULL)
                    continue;
                
                __CUIControllerEnumerateCredentialsExcepting(_controller,
                                                             attrs,
                                                             kKeychainCredentialProviderFactoryID,
                                                             ^(CUICredentialRef cred, CFErrorRef err) {
                                                                 CUIKeychainCredential *itemCred;
                                                                 
                                                                 if (cred == NULL)
                                                                     return;
                                                                 
                                                                 itemCred = new CUIKeychainCredential();
                                                                 if (!itemCred->initWithCredential(cred, _usageFlags)) {
                                                                     itemCred->Release();
                                                                     return;
                                                                 }
                                                                 
                                                                 CFArrayAppendValue(creds, itemCred);
                                                                 itemCred->Release();
                                                             });
                
                CFRelease(attrs);
            }
            
            CFRelease(items);
        }
        
        return creds;
    }
    
    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error) {
        if ((usageFlags & kCUIUsageFlagsGeneric) == 0 ||
            (usageFlags & (kCUIUsageFlagsInCredOnly | kCUIUsageFlagsExcludePersistedCreds)))
            return false;
        
        _controller = (CUIControllerRef)CFRetain(controller);
        _usageScenario = usageScenario;
        _usageFlags = usageFlags;
        
        return true;
    }
    
    CUIKeychainCredentialProvider() {
        CFPlugInAddInstanceForFactory(kKeychainCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
        _usageScenario = kCUIUsageScenarioInvalid;
        _usageFlags = 0;
    }
    
protected:
    
    ~CUIKeychainCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kKeychainCredentialProviderFactoryID);
    }
    
};

__attribute__((visibility("default"))) void *
CUIKeychainCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIKeychainCredentialProvider;
    
    return NULL;
}
