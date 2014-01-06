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
    
    Boolean initWithCredential(CUICredentialRef credential, CUIUsageFlags usageFlags) {
        if (credential == NULL)
            return false;
        
        _credential = (CUICredentialRef)CFRetain(credential);
        
        _item = (GSSItemRef)CFDictionaryGetValue(getAttributes(), kCUIAttrGSSItemRef);
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
        CUICredentialDidSubmit(_credential);
    }
 
    CFDictionaryRef copyItemAttributes(void);
    Boolean savePersisted(CFErrorRef *error);
    Boolean deletePersisted(CFErrorRef *error);
    
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
