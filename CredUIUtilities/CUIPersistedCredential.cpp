//
//  CUIPersistedCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 15/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPersistedCredential.h"

class CUIPersistedCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CUIPersistedCredential %p{credential = \"%@\"}>"),
                                        this, _credential);
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return CUICredentialGetFields(_credential);
    }
    
    CFDictionaryRef getAttributes(void) {
        return CUICredentialGetAttributes(_credential);
    }
    
    Boolean initWithPersistenceProviderAndCredential(CUICredentialPersistenceEx *persistence, CUICredentialRef credential) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);
        
        _persistence = persistence;
        _persistence->AddRef();
        
        return true;
    }
    
    void didBecomeSelected(void) {
        CUICredentialDidBecomeSelected(_credential);
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
        
        return _persistence->updateCredential(_credential, error);
    }
    
    Boolean deletePersisted(CFErrorRef *error) {
        if (!CUICredentialDeletePersisted(_credential, error))
            return false;
        
        return _persistence->deleteCredential(_credential, error);
    }
    
    CUIPersistedCredential() {
        _retainCount = 1;
        _credential = NULL;
    }
    
private:
    int32_t _retainCount;
    CUICredentialRef _credential;
    CUICredentialPersistenceEx *_persistence;
    
protected:
    ~CUIPersistedCredential() {
        if (_credential)
            CFRelease(_credential);
        if (_persistence)
            _persistence->Release();
    }
};

CUICredentialRef
CUIPersistedCredentialCreate(CUICredentialPersistenceEx *persistence, CUICredentialRef cred)
{
    CUIPersistedCredential *itemCred;
    CUICredentialRef credRef;
    
    itemCred = new CUIPersistedCredential();
    if (!itemCred->initWithPersistenceProviderAndCredential(persistence, cred)) {
        itemCred->Release();
        return NULL;
    }
    
    credRef = CUICredentialCreate(CFGetAllocator(cred), itemCred);
    if (credRef == NULL) {
        itemCred->Release();
        return NULL;
    }
    
    itemCred->Release();

    return credRef;
}
