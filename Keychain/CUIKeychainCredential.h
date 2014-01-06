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

#include <CredUICore/CredUICore.h>
#include <CredUICore/CredUICore_Private.h>

#include <Security/Security.h>

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
    
    Boolean initWithCredential(CUICredentialRef credential, CUIUsageFlags usageFlags) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);

        _item = (SecKeychainItemRef)CFDictionaryGetValue(getAttributes(), kCUIAttrSecKeychainItemRef);
        if (_item == NULL)
            return false;
        CFRetain(_item);

        _usageFlags = usageFlags;
        
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
    
    CFDictionaryRef createQuery(void) {
        CFMutableDictionaryRef query;
      
        query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (query == NULL)
            return NULL;
        
        CFDictionarySetValue(query, kSecValueRef, _item);
        
        return query;
    }
    
    CFMutableDictionaryRef copyKeychainAttributes(void) {
        Boolean bCUIGeneric;
        return CUICreateKeychainAttributesFromCUIAttributes(getAttributes(), NULL, &bCUIGeneric);
    }
    
    Boolean savePersisted(CFErrorRef *error) {
        if (!CUICredentialSavePersisted(_credential, error))
            return false;

        Boolean ret = false;
        CFMutableDictionaryRef keychainAttrs = copyKeychainAttributes();
        CFDictionaryRef query = createQuery();
            
        if (keychainAttrs && query &&
            CUIKeychainSetPasswordAttr(keychainAttrs, getAttributes())) {
            ret = (SecItemUpdate(query, keychainAttrs) == errSecSuccess);
        } else {
            ret = false;
        }
        
        if (query)
            CFRelease(query);
        if (keychainAttrs)
            CFRelease(keychainAttrs);
        
        return ret;
    }
    
    Boolean deletePersisted(CFErrorRef *error) {
        if (!CUICredentialDeletePersisted(_credential, error))
            return false;
        
        Boolean ret;
        CFDictionaryRef query;
        
        query = createQuery();
        if (query == NULL)
            return false;
        
        ret = (SecItemDelete(query) == errSecSuccess);
        
        CFRelease(query);
        
        return ret;
    }
    
    CUIKeychainCredential() {
        _retainCount = 1;
        _item = NULL;
        _credential = NULL;
        _usageFlags = 0;
    }
    
private:
    int32_t _retainCount;
    CUICredentialRef _credential;
    SecKeychainItemRef _item;
    CUIUsageFlags _usageFlags;
    
protected:
    ~CUIKeychainCredential() {
        if (_credential)
            CFRelease(_credential);
        if (_item)
            CFRelease(_item);
    }
};
#endif /* defined(__CredUI__CUIKeychainCredential__) */
