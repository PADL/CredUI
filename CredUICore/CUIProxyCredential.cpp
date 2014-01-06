//
//  CUIProxyCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#include <libkern/OSAtomic.h>

/*
 * Proxy credential for future out of process usage
 */

class CUIProxyCredential : public CUICredentialContext {
    
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
                                        CFSTR("<CUIProxyCredential %p{name = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kCUIAttrName));
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return NULL;
    }
    
    CFDictionaryRef getAttributes(void) {
        return _attributes;
    }
    
    Boolean initWithAttributes(CFDictionaryRef attributes) {
        if (attributes)
            _attributes = (CFDictionaryRef)CFRetain(attributes);
        
        return !!_attributes;
    }
    
    void didBecomeSelected(Boolean *pbAutoLogin) {}
    void didBecomeDeselected(void) {}
    void didSubmit(void) {}
   
    Boolean savePersisted(CFErrorRef *) {
        return false;
    }
    
    Boolean deletePersisted(CFErrorRef *) {
        return false;
    }

    CUIProxyCredential() {
        _retainCount = 1;
        _attributes = NULL;
    }
    
private:
    int32_t _retainCount;
    CFDictionaryRef _attributes;
    
protected:
    
    ~CUIProxyCredential() {
        if (_attributes)
            CFRelease(_attributes);
    }
};

CUI_EXPORT CUICredentialRef
CUICredentialCreateProxy(CFAllocatorRef allocator,
                         CFDictionaryRef credAttributes)
{
    CUIProxyCredential *proxyCredContext;
    CUICredentialRef proxyCred;
    
    proxyCredContext = new CUIProxyCredential();
    if (proxyCredContext == NULL)
        return NULL;
    
    if (!proxyCredContext->initWithAttributes(credAttributes)) {
        proxyCredContext->Release();
        return NULL;
    }
    
    proxyCred = CUICredentialCreate(allocator, proxyCredContext);
    proxyCredContext->Release();
    
    return proxyCred;
}
