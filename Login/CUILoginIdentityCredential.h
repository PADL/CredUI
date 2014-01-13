//
//  CUILoginIdentityCredential.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUILoginIdentityCredential__
#define __CredUI__CUILoginIdentityCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <Security/Security.h>

#include <CredUICore/CredUICore.h>

#include "CUILoginIdentityCredentialProvider.h"
#include "CUIProviderUtilities.h"

class CUILoginIdentityCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CUILoginIdentityCredential %p{credential = \"%@\"}>"),
                                        this, _credential);
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return CUICredentialGetFields(_credential);
    }
    
    CFDictionaryRef getAttributes(void) {
        return CUICredentialGetAttributes(_credential);
    }
    
    Boolean initWithCredential(CUICredentialRef credential) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);
        
        return true;
    }
    
    void didBecomeSelected(Boolean *pbAutoLogin) {
        CUICredentialDidBecomeSelected(_credential, pbAutoLogin);
    }
    
    void didBecomeDeselected(void) {
        CUICredentialDidBecomeDeselected(_credential);
    }
    
    void didSubmit(void) {
        CUICredentialDidSubmit(_credential);
    }
    
    Boolean savePersisted(CFErrorRef *error) {
        return CUICredentialSavePersisted(_credential, error);
    }
    
    Boolean deletePersisted(CFErrorRef *error) {
        return CUICredentialDeletePersisted(_credential, error);
    }
    
    CUILoginIdentityCredential() {
        _retainCount = 1;
        _credential = NULL;
    }
    
private:
    int32_t _retainCount;
    CUICredentialRef _credential;
    
protected:
    ~CUILoginIdentityCredential() {
        if (_credential)
            CFRelease(_credential);
    }
};
#endif /* defined(__CredUI__CUILoginIdentityCredential__) */
