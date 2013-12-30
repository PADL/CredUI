//
//  CPasswordCredential.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CPasswordCredential__
#define __CredUI__CPasswordCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include "CredUICore.h"

class CPasswordCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CPasswordCredential %p{attributes = \"%@\"}>"), this, _attributes);
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return _fields;
    }
    
    CFDictionaryRef getAttributes(void) {
        return _attributes;
    }
    
    Boolean initWithAttributes(CFDictionaryRef attributes);
 
    void didBecomeSelected(Boolean *pbAutoLogin) {
        *pbAutoLogin = false;
    }
    
    void didBecomeDeselected(void) {
    }
    
    CPasswordCredential() {
        _retainCount = 1;
        _fields = NULL;
        _attributes = NULL;
    }

private:
    int32_t _retainCount;
    CFArrayRef _fields;
    CFMutableDictionaryRef _attributes;
    
protected:
    
    ~CPasswordCredential() {
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
    }
};

#endif /* defined(__CredUI__CPasswordCredential__) */
