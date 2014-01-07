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

        return _attributes;
    }
    
    Boolean initWithControllerAndAttributes(CUIControllerRef controller,
                                            CUIUsageFlags usageFlags,
                                            CFDictionaryRef attributes,
                                            CFErrorRef *error);
 
    const CFStringRef getCredentialStatus(void);

    void didBecomeSelected(Boolean *pbAutoLogin) {
        *pbAutoLogin = false;
#if 0
        CUIFieldSetOptions((CUIFieldRef)CFArrayGetValueAtIndex(_fields, 2), kCUIFieldOptionsIsHidden);
#endif
    }
    
    void didBecomeDeselected(void) {
    }
    
    void didSubmit(void) {
   }
    
    void syncPersistedPassword(void);
    
    Boolean hasPlaceholderPassword(void) {
        CFTypeRef password = CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);
       
        return password && CFEqual(password, kCFBooleanTrue);
    }

    Boolean hasNonPlaceholderPassword(void) {
        CFTypeRef password = CFDictionaryGetValue(_attributes, kCUIAttrCredentialPassword);

        if (password == NULL)
            return false;

        /* we're going to allow passwords to be CFDataRefs for future use */
        return (CFGetTypeID(password) == CFStringGetTypeID() && CFStringGetLength((CFStringRef)password)) ||
               (CFGetTypeID(password) == CFDataGetTypeID()   && CFDataGetLength((CFDataRef)password));
    }
   
    Boolean hasPassword(void) {
        return hasPlaceholderPassword() || hasNonPlaceholderPassword();
    }
 
    Boolean savePersisted(CFErrorRef *error);
    
    Boolean deletePersisted(CFErrorRef *error) {
        return true;
    }
    
    CUIPasswordCredential() {
        _retainCount = 1;
        _fields = NULL;
        _attributes = NULL;
        _controller = NULL;
    }

private:
    int32_t _retainCount;
    bool _generic;
    CFArrayRef _fields;
    CFMutableDictionaryRef _attributes;
    CUIControllerRef _controller;
    
protected:
    
    ~CUIPasswordCredential() {
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
        if (_controller)
            CFRelease(_controller);
    }
};

#endif /* defined(__CredUI__CUIPasswordCredential__) */
