//
//  GSSItemCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CredUICore_Private.h>

#include <GSS/GSS.h>
#include "CUIProviderUtilities.h"

#include "GSSItem.h"
#include "CUIGSSItemCredentialProvider.h"
#include "CUIGSSItemCredential.h"


// 2F62D1C1-F586-41CC-8096-C90683068DA5
#define kGSSItemCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x2F, 0x62, 0xD1, 0xC1, 0xF5, 0x86, 0x41, 0xCC, 0x80, 0x96, 0xC9, 0x06, 0x83, 0x06, 0x8D, 0xA5)

extern "C" {
    void *CUIGSSItemCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

class CUIGSSItemCredentialProvider : public CUIProvider {
    
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
        CFDictionaryRef gssItemAttributes = NULL;
        CFArrayRef items;
        CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                       0,
                                                       &kCFTypeArrayCallBacks);

        if (attributes)
            gssItemAttributes = CUICreateGSSItemAttributesFromCUIAttributes(attributes);
                                                                              
        items = GSSItemCopyMatching(gssItemAttributes, error);
        if (items) {
            for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
                GSSItemRef item = (GSSItemRef)CFArrayGetValueAtIndex(items, index);
                CFMutableDictionaryRef cuiAttributes = CUICreateCUIAttributesFromGSSItemAttributes(item->keys); // XXX private data
                
                if (cuiAttributes == NULL)
                    continue;
                
                CFDictionarySetValue(cuiAttributes, kCUIAttrGSSItemRef, item);
                
                __CUIControllerEnumerateCredentialsExcepting(_controller,
                                                             cuiAttributes,
                                                             kGSSItemCredentialProviderFactoryID,
                                                             ^(CUICredentialRef cred, CFErrorRef err) {
                     CUIGSSItemCredential *itemCred;
                     CUICredentialRef credRef;
                                                                 
                     if (cred == NULL)
                         return;
                     
                     itemCred = new CUIGSSItemCredential();
                     if (!itemCred->initWithCredential(cred, _usageFlags)) {
                         itemCred->Release();
                         return;
                     }
                         
                     credRef = CUICredentialCreate(CFGetAllocator(_controller), itemCred);
                     if (credRef == NULL) {
                         itemCred->Release();
                         return;
                     }
                     
                     CFArrayAppendValue(creds, credRef);

                     CFRelease(credRef);
                     itemCred->Release();
                 });
                
                CFRelease(cuiAttributes);
            }
            
            CFRelease(items);
        }
        
        if (gssItemAttributes)
            CFRelease(gssItemAttributes);
        
        return creds;
    }
    
    Boolean initWithController(CUIControllerRef controller,
                               CUIUsageScenario usageScenario,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error) {
        if (usageFlags & (kCUIUsageFlagsGeneric | kCUIUsageFlagsInCredOnly | kCUIUsageFlagsExcludePersistedCreds))
            return false;
        
        _controller = (CUIControllerRef)CFRetain(controller);
        _usageScenario = usageScenario;
        _usageFlags = usageFlags;
        
        return true;
    }
    
    CUIGSSItemCredentialProvider() {
        CFPlugInAddInstanceForFactory(kGSSItemCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
        _usageScenario = kCUIUsageScenarioInvalid;
        _usageFlags = 0;
    }
    
protected:
    
    ~CUIGSSItemCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kGSSItemCredentialProviderFactoryID);
    }
    
};

__attribute__((visibility("default"))) void *
CUIGSSItemCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIGSSItemCredentialProvider;
    
    return NULL;
}
