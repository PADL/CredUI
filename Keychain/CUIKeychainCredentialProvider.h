//
//  CUIKeychainCredentialProvider.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIKeychainCredentialProvider__
#define __CredUI__CUIKeychainCredentialProvider__

#include "CUIProviderUtilities.h"
#include "CUIPersistedCredential.h"

class CUIKeychainCredentialProvider : public CUIProvider, public CUICredentialPersistenceEx {
    
private:
    int32_t _retainCount;
    CUIControllerRef _controller;
    
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
        
        if (CFEqual(interfaceID, kCUIProviderInterfaceID) ||
            CFEqual(interfaceID, IUnknownUUID))
            *ppv = (CUIProvider *)this;
        else if (CFEqual(interfaceID, kCUIPersistenceInterfaceID))
            *ppv = (CUICredentialPersistence *)this;
        else
            *ppv = NULL;
        
        if (*ppv != NULL)
            AddRef();
        
        CFRelease(interfaceID);
        
        return *ppv ? S_OK : E_NOINTERFACE;
    }
    
    Boolean initWithController(CUIControllerRef controller, CFErrorRef *error);
    
    CFArrayRef copyMatchingCredentials(CFDictionaryRef attributes,
                                       CUIUsageFlags usageFlags,
                                       CFIndex *defaultCredentialIndex,
                                       CFErrorRef *error);
    CFSetRef getWhitelistedAttributeKeys(void) {
        return NULL;
    }

    Boolean addCredentialWithAttributes(CFDictionaryRef attributes, CFErrorRef *error);
    CFTypeRef extractPassword(CFDictionaryRef attributes, CFErrorRef *error);

    Boolean updateCredential(CUICredentialRef credential, CFErrorRef *error);
    Boolean deleteCredential(CUICredentialRef credential, CFErrorRef *error);
 
    /* helpers */ 
    static CFMutableDictionaryRef createQuery(CFDictionaryRef attributes);
    static CFMutableDictionaryRef createCUIAttributesFromKeychainAttributes(CFDictionaryRef keychainAttrs);
    static CFMutableDictionaryRef createKeychainAttributesFromCUIAttributes(CFDictionaryRef attributes, CFTypeRef targetName);
    static CFArrayRef copyMatching(CFDictionaryRef attributes, CFTypeRef targetName, CFErrorRef *error);
    static Boolean setPasswordAttr(CFMutableDictionaryRef keychainAttrs, CFDictionaryRef attributes);

    CUIKeychainCredentialProvider() {
        CFPlugInAddInstanceForFactory(kKeychainCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
    }
    
protected:
    
    ~CUIKeychainCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kKeychainCredentialProviderFactoryID);
    }
    
};

#endif /* defined(__CredUI__CUIKeychainCredentialProvider__) */
