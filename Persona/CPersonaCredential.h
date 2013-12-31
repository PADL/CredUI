//
//  CPersonaCredential.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CPersonaCredential__
#define __CredUI__CPersonaCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <browserid.h>
#include <CFBrowserID.h>

#include "CredUICore.h"

class CPersonaCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CPersonaCredential %p{name = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kGSSAttrName));
        
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

    Boolean initWithControllerAndAttributes(
        CUIControllerRef controller,
        CFDictionaryRef attributes,
        CFErrorRef *error);
    
    void didBecomeSelected(Boolean *pbAutoLogin) {}
    void didBecomeDeselected(void) {}
    void didSubmit(void);
    
    CPersonaCredential() {
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
    ~CPersonaCredential() {
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

#endif /* defined(__CredUI__CPersonaCredential__) */
