//
//  CUIPasswordCredential.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIPasswordCredential__
#define __CredUI__CUIPasswordCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include "CredUICore.h"

class CUIPasswordCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CUIPasswordCredential %p{name = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kCUIAttrName));
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return _fields;
    }
    
    CFDictionaryRef getAttributes(void) {
        CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, getCredentialStatus());
        CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, CFSTR("PasswordCredentialProvider"));

        return _attributes;
    }
    
    Boolean initWithAttributes(CFDictionaryRef attributes,
                               CUIUsageFlags usageFlags,
                               CFErrorRef *error);
 
    const CFStringRef getCredentialStatus(void);

    void didBecomeSelected(Boolean *pbAutoLogin) {
        *pbAutoLogin = false;
    }
    
    void didBecomeDeselected(void) {
    }
    
    void didSubmit(void) {
    }
   
    Boolean confirm(CFErrorRef *error) {
        return false;
    }

    CUIPasswordCredential() {
        _retainCount = 1;
        _inCredUsable = false;
        _fields = NULL;
        _attributes = NULL;
    }

private:
    int32_t _retainCount;
    bool _inCredUsable;
    CFArrayRef _fields;
    CFMutableDictionaryRef _attributes;
    
protected:
    
    ~CUIPasswordCredential() {
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
    }
};

#endif /* defined(__CredUI__CUIPasswordCredential__) */
