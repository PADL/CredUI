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
    
    CFDictionaryRef copyItemAttributes(void) {
        return CUICreateGSSItemAttributesFromCUIAttributes(getAttributes());
    }
    
    Boolean savePersisted(CFErrorRef *error) {
       CFDictionaryRef gssItemAttributes;
       Boolean ret;
       
        /*
         * We update any existing GSS items on behalf of the credential provider, however
         * adding any new ones must be done by the credential provider themselves.
         */ 
        ret = CUICredentialSavePersisted(_credential, error);
        if (!ret)
            return ret;
        
        if (_item) {
            gssItemAttributes = copyItemAttributes();
            if (gssItemAttributes == NULL)
                return false;
            
            ret = GSSItemUpdate(_item->keys, gssItemAttributes, error);
            CFRelease(gssItemAttributes);
        }
        
        return ret;
    }
    
    Boolean deletePersisted(CFErrorRef *error) {
        CFDictionaryRef gssItemAttributes;
        Boolean ret;

        ret = CUICredentialDeletePersisted(_credential, error);
        if (!ret)
            return ret;
        
        gssItemAttributes = copyItemAttributes();
        if (gssItemAttributes == NULL)
            return false;
        
        ret = GSSItemDelete(gssItemAttributes, error);
        CFRelease(gssItemAttributes);
        
        return ret;
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
