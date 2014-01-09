//
//  CUIKeychainCredential.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIKeychainCredential__
#define __CredUI__CUIKeychainCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <Security/Security.h>

#include <CredUICore/CredUICore.h>

#include "CUIKeychainCredentialProvider.h"
#include "CUIProviderUtilities.h"

class CUIKeychainCredential : public CUICredentialContext {
    
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
        if (CFEqual(interfaceID, kCUICredentialInterfaceID) ||
            CFEqual(interfaceID, IUnknownUUID)) {
            *ppv = this;
            AddRef();
            CFRelease(interfaceID);
            return S_OK;
        }
        CFRelease(interfaceID);
        return E_NOINTERFACE;
    }
    
    CFStringRef copyDescription(void) {
        CFStringRef desc;
        
        desc = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
                                        CFSTR("<CUIKeychainCredential %p{credential = \"%@\"}>"),
                                        this, _credential);
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return CUICredentialGetFields(_credential);
    }
    
    CFDictionaryRef getAttributes(void) {
        return CUICredentialGetAttributes(_credential);
    }
    
    Boolean initWithCredential(CUICredentialRef credential, CUIKeychainCredentialProvider *provider) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);
        
        _provider = provider;
        _provider->AddRef();
        
        return true;
    }
    
    void didBecomeSelected(Boolean *pbAutoLogin) {
        CUICredentialDidBecomeSelected(_credential, pbAutoLogin);
    }
    
    void didBecomeDeselected(void) {
        CUICredentialDidBecomeDeselected(_credential);
    }
    
    void didSubmit(void) {
        // now, unlock the keychain item
        
        CUICredentialDidSubmit(_credential);
    }
    
    Boolean savePersisted(CFErrorRef *error) {
        if (!CUICredentialSavePersisted(_credential, error))
            return false;

        return _provider->updateCredential(_credential, error);
    }
    
    Boolean deletePersisted(CFErrorRef *error) {
        if (!CUICredentialDeletePersisted(_credential, error))
            return false;
        
        return _provider->deleteCredential(_credential, error);
    }
    
    CUIKeychainCredential() {
        _retainCount = 1;
        _credential = NULL;
    }
    
private:
    int32_t _retainCount;
    CUICredentialRef _credential;
    CUIKeychainCredentialProvider *_provider;
    
protected:
    ~CUIKeychainCredential() {
        if (_credential)
            CFRelease(_credential);
        if (_provider)
            _provider->Release();
    }
};
#endif /* defined(__CredUI__CUIKeychainCredential__) */
