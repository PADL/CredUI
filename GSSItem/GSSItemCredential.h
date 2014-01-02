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

#include "GSSItem.h"
#include "GSSItemUtilities.h"

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
    
    Boolean initWithItemAndCredential(GSSItemRef item,
                                      CUICredentialRef credential,
                                      CUIUsageFlags usageFlags) {
        if (item == NULL || credential == NULL)
            return false;
        
        _item = (GSSItemRef)CFRetain(item);
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
        CUICredentialDidSubmit(_credential);
    }
    
    Boolean confirm(CFErrorRef *error) {
        Boolean ret;
        
        ret = CUICredentialConfirm(_credential, error);
        if (ret && _item) {
            CFDictionaryRef gssItemAttributes = GSSItemUtilities::createGSSItemAttributes(getAttributes());
        
            if (gssItemAttributes) {
                ret = GSSItemUpdate(_item->keys, gssItemAttributes, error);
                CFRelease(gssItemAttributes);
            }
        }
        
        return true;
    }
    
    CUIGSSItemCredential() {
        _retainCount = 1;
        _item = NULL;
        _credential = NULL;
        _usageFlags = 0;
    }
    
private:
    int32_t _retainCount;
    GSSItemRef _item;
    CUICredentialRef _credential;
    CUIUsageFlags _usageFlags;
    
protected:
    ~CUIGSSItemCredential() {
        if (_item)
            CFRelease(_item);
        if (_credential)
            CFRelease(_credential);
    }
};
#endif /* defined(__CredUI__GSSItemCredential__) */
