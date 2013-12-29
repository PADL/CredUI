//
//  CUIProvider.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIProvider__
#define __CredUI__CUIProvider__

#include <CoreFoundation/CFPlugInCOM.h>

// BFA3619B-1A12-4DBA-801F-33B0874DD76F
#define kCUIProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xBF, 0xA3, 0x61, 0x9B, 0x1A, 0x12, 0x4D, 0xBA, 0x80, 0x1F, 0x33, 0xB0, 0x87, 0x4D, 0xD7, 0x6F)

// F7356A4B-91A7-4455-AF3A-175D27449C9E
#define kCUIProviderInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xF7, 0x35, 0x6A, 0x4B, 0x91, 0xA7, 0x44, 0x55, 0xAF, 0x3A, 0x17, 0x5D, 0x27, 0x44, 0x9C, 0x9E)

// 2C8CF116-62BA-44B6-B894-C6A592E62566
#define kCUIProviderTypeID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x2C, 0x8C, 0xF1, 0x16, 0x62, 0xBA, 0x44, 0xB6, 0xB8, 0x94, 0xC6, 0xA5, 0x92, 0xE6, 0x25, 0x66)

// 20C3A840-6BC4-4B26-B705-43B4C46218E7
#define kCUICredentialInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x20, 0xC3, 0xA8, 0x40, 0x6B, 0xC4, 0x4B, 0x26, 0xB7, 0x05, 0x43, 0xB4, 0xC4, 0x62, 0x18, 0xE7)

extern CFArrayCallBacks kCUICredentialContextArrayCallBacks;

#ifdef __cplusplus
class CUICredentialContext : public IUnknown {
public:
    /*
     * Return a description for debugging purposes
     */
    virtual CFStringRef copyDescription(void) = 0;
    
    /*
     * Get an array of CUIFieldRefs that can be used to populate the
     * auth identity.
     */
    virtual CFArrayRef getFields(void) = 0;
    
    /*
     * Pack the fields into a GSS auth identity.
     */
    virtual CFDictionaryRef getAttributes(void) = 0;
    
    /*
     * Called when the user selects.
     */
    virtual void didBecomeSelected(void) = 0;
};

class CUIProvider : public IUnknown {
public:
    /*
     * Initialize a new credential provider
     */
    virtual Boolean initWithController(CUIControllerRef controller) = 0;
    
    /*
     * Get a CUICredentialRef for an authentication identity. attribuets may be
     * NULL in which case it should prompt the user.
     */
    virtual CUICredentialContext *getCredentialWithAttributes(CFDictionaryRef attributes) = 0;
    
    /*
     * Get any additional CUICredentialContexts for users that the provider knows about,
     * for example inserted smartcards.
     */
    virtual CFArrayRef getOtherCredentials(void) = 0; // array of CUICredentialRef
};

#else
typedef struct CUICredentialContext {
    IUNKNOWN_C_GUTS;
    CFStringRef (STDMETHODCALLTYPE *copyDescription)(void *thisPointer);
    CFArrayRef (STDMETHODCALLTYPE *getFields)(void *thisPointer);
    CFDictionaryRef (STDMETHODCALLTYPE *getAttributes)(void *thisPointer);
    void (STDMETHODCALLTYPE *didBecomeSelected)(void *thisPointer);
} CUICredentialContext;

typedef struct CUIProvider {
    IUNKNOWN_C_GUTS;
    HRESULT (STDMETHODCALLTYPE *initWithController)(CUIControllerRef controller);
    CUICredentialRef (STDMETHODCALLTYPE *getCredentialWithAttributes)(CFDictionaryRef attributes);
    CFArrayRef (STDMETHODCALLTYPE *getOtherCredentials)(void *thisPointer);
} CUIProvider;
#endif /* defined(__cplusplus) */

#endif /* defined(__CredUI__CUIProvider__) */
