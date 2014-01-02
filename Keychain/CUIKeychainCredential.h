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
    
    Boolean initWithItemAndCredential(SecKeychainItemRef item,
                                      CUICredentialRef credential,
                                      CFTypeRef targetName,
                                      CUIUsageFlags usageFlags) {
        if (item == NULL || credential == NULL)
            return false;
        
        _item = (SecKeychainItemRef)CFRetain(item);
        _credential = (CUICredentialRef)CFRetain(credential);
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
    
    Boolean didConfirm(CFErrorRef *error) {
        Boolean ret;
        
        ret = CUICredentialDidConfirm(_credential, error);
        if (ret) {
            Boolean bCUIGeneric;
            CFMutableDictionaryRef query;
            CFMutableDictionaryRef keychainAttrs;
            OSStatus osret;
            
            query = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            if (query == NULL)
                return false;
            
            CFDictionarySetValue(query, kSecValueRef, _item);
            
            // we don't set targetName because we don't want to clobber that
            keychainAttrs = CUICreateKeychainAttributesFromCUIAttributes(getAttributes(), NULL, &bCUIGeneric);
            if (keychainAttrs == NULL) {
                CFRelease(query);
                return false;
            }
 
            if (!CUIKeychainSetPasswordAttr(keychainAttrs, getAttributes())) {
                CFRelease(query);
                CFRelease(keychainAttrs);
                return false;
            }

            osret = SecItemUpdate(query, keychainAttrs);
            
            CFRelease(query);
            CFRelease(keychainAttrs);
            
            ret = !osret;
        }
        
        return ret;
    }
    
    CUIKeychainCredential() {
        _retainCount = 1;
        _targetName = NULL;
        _item = NULL;
        _credential = NULL;
        _targetName = NULL;
        _usageFlags = 0;
    }
    
private:
    int32_t _retainCount;
    CFTypeRef _targetName;
    SecKeychainItemRef _item;
    CUICredentialRef _credential;
    CUIUsageFlags _usageFlags;
    
protected:
    ~CUIKeychainCredential() {
        if (_item)
            CFRelease(_item);
        if (_credential)
            CFRelease(_credential);
        if (_targetName)
            CFRelease(_targetName);
    }
};
#endif /* defined(__CredUI__CUIKeychainCredential__) */
