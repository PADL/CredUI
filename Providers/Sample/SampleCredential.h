//
//  SampleCredential.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__SampleCredential__
#define __CredUI__SampleCredential__

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>

class SampleCredential : public CUICredentialContext {
    
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
   
    /*
     * A human readable description of the credential for debugging.
     */ 
    CFStringRef copyDescription(void) {
        CFStringRef desc;
        
        desc = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
                                        CFSTR("<SampleCredential %p{name = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kCUIAttrName));
        
        return desc;
    }
   
    /*
     * The fields to be rendered by the UI.
     */ 
    CFArrayRef getFields(void) {
        return _fields;
    }
  
    /*
     * The attributes serialized from the completed fields.
     */ 
    CFDictionaryRef getAttributes(void) {
        return _attributes;
    } 

    /*
     * Credential seleciton/submission notifications
     */ 
    void didBecomeSelected(void) {
    }
    
    void didBecomeDeselected(void) {
    }
    
    void didSubmit(void) {
    }
    
    void savePersisted(void (^completionHandler)(CFErrorRef)) {
        completionHandler(NULL);
    }
    
    void deletePersisted(void (^completionHandler)(CFErrorRef)) {
        completionHandler(NULL);
    }

    /*
     * Helper methods for this class
     */   
    void setUsername(CFStringRef username);
    CFStringRef getDefaultUsername(void);

    void updateCredentialStatus(void);

    Boolean initWithControllerAndAttributes(CUIControllerRef controller,
                                            CUIUsageFlags usageFlags,
                                            CFDictionaryRef attributes,
                                            CFErrorRef *error);
   
     /*
     * Constructor
     */    
    SampleCredential() {
        _retainCount = 1;
        _fields = NULL;
        _attributes = NULL;
    }
    
private:
    int32_t _retainCount;
    CFArrayRef _fields;
    CFMutableDictionaryRef _attributes;
    
protected:
    
    ~SampleCredential() {
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
    }
};

#endif /* defined(__CredUI__SampleCredential__) */
