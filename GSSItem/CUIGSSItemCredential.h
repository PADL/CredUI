//
//  GSSItemCredential.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__GSSItemCredential__
#define __CredUI__GSSItemCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CredUICore_Private.h>

#include "CUIProviderUtilities.h"
#include "GSSItem.h"
#include "CUIGSSItemCredentialProvider.h"

/*
 * Return an attribute dictionary suitable for passing to a GSS item API
 * from a CredUI attribute dictionary.
 */
extern CFMutableDictionaryRef
CUICreateCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes);

/*
 * Return an attribute dictionary suitable for passing to a CredUI API
 * from a GSS item attribute dictionary.
 */
extern CFMutableDictionaryRef
CUICreateGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes);

class CUIGSSItemCredential : public CUICredentialContext {
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
                                        CFSTR("<CUIGSSItemCredential %p{credential = \"%@\"}>"),
                                        this, _credential);

        return desc;
    }
    
    CFArrayRef getFields(void) {
        return CUICredentialGetFields(_credential);
    }
    
    CFDictionaryRef getAttributes(void) {
        return CUICredentialGetAttributes(_credential);
    }
    
    Boolean initWithCredential(CUICredentialRef credential, CUIUsageFlags usageFlags, CUIGSSItemCredentialProvider *provider) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);
        _usageFlags = usageFlags;
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

    CUIGSSItemCredential() {
        _retainCount = 1;
        _credential = NULL;
        _usageFlags = 0;
    }
    
private:
    int32_t _retainCount;
    CUICredentialRef _credential;
    CUIUsageFlags _usageFlags;
    CUIGSSItemCredentialProvider *_provider;
    
protected:
    ~CUIGSSItemCredential() {
        if (_credential)
            CFRelease(_credential);
    }
};
#endif /* defined(__CredUI__GSSItemCredential__) */
