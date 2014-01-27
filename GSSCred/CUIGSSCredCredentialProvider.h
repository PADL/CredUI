//
//  CUIGSSCredCredentialProvider.h
//  CredUI
//
//  Created by Luke Howard on 21/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIGSSCredCredentialProvider__
#define __CredUI__CUIGSSCredCredentialProvider__

#include <CoreFoundation/CoreFoundation.h>
#include <GSS/GSS.h>
#include <libkern/OSAtomic.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CUIProvider.h>

// A02B6687-7AD5-4EE9-8BB5-95974264C37D
#define kGSSCredCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xA0, 0x2B, 0x66, 0x87, 0x7A, 0xD5, 0x4E, 0xE9, 0x8B, 0xB5, 0x95, 0x97, 0x42, 0x64, 0xC3, 0x7D)

class CUIGSSCredCredentialProvider : public CUIProvider {
    
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
    
    CFDictionaryRef copyAttributesForCredential(gss_cred_id_t cred);

    CUIGSSCredCredentialProvider() {
        CFPlugInAddInstanceForFactory(kGSSCredCredentialProviderFactoryID);
        _retainCount = 1;
        _controller = NULL;
    }
    
protected:
    
    ~CUIGSSCredCredentialProvider() {
        if (_controller)
            CFRelease(_controller);
        CFPlugInRemoveInstanceForFactory(kGSSCredCredentialProviderFactoryID);
    }
    
};

#endif /* defined(__CredUI__CUIGSSCredCredentialProvider__) */
