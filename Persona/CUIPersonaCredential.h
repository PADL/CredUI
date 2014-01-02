//
//  CUIPersonaCredential.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIPersonaCredential__
#define __CredUI__CUIPersonaCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <browserid.h>
#include <CFBrowserID.h>

#include "CredUICore.h"

class CUIPersonaCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CUIPersonaCredential %p{identity = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kCUIAttrCredentialBrowserIDIdentity));
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return _fields;
    }
    
    CFDictionaryRef getAttributes(void) {
        return _attributes;
    }

    Boolean createBrowserIDContext(CUIControllerRef controller, CFErrorRef *error);
    Boolean createBrowserIDAssertion(CFErrorRef *error);

    Boolean initWithControllerAndAttributes(CUIControllerRef controller,
                                            CUIUsageFlags usageFlags,
                                            CFDictionaryRef attributes,
                                            CFErrorRef *error);
    
    void didBecomeSelected(Boolean *pbAutoLogin) {
        *pbAutoLogin = true;
    }
    void didBecomeDeselected(void) {}
    void didSubmit(void) {}
 
    Boolean didConfirm(CFErrorRef *error) {
        return false;
    }

    CUIPersonaCredential() {
        _retainCount = 1;
        _bidContext = NULL;
        _defaultIdentity = NULL;
        _targetName = NULL;
        _fields = NULL;
        _attributes = NULL;
    }

private:
    int32_t _retainCount;
    BIDContext _bidContext;
    CFStringRef _defaultIdentity;
    CFStringRef _targetName;
    CFArrayRef _fields;
    CFMutableDictionaryRef _attributes;
    
protected:
    ~CUIPersonaCredential() {
        if (_bidContext)
            CFRelease(_bidContext);
        if (_defaultIdentity)
            CFRelease(_defaultIdentity);
        if (_targetName)
            CFRelease(_targetName);
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
    }
};

#endif /* defined(__CredUI__CUIPersonaCredential__) */
