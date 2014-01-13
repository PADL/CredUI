//
//  CUIProvider.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUIProvider_h
#define CredUICore_CUIProvider_h

#include <CoreFoundation/CFPlugInCOM.h>

#ifdef __cplusplus
class CUICredentialContext : public IUnknown {
public:
    /*
     * Return a description for debugging purposes
     */
    virtual CFStringRef copyDescription(void) CF_RETURNS_RETAINED = 0;
    
    /*
     * Get an array of CUIFieldRefs that can be used to populate the
     * auth identity.
     */
    virtual CFArrayRef getFields(void) CF_RETURNS_NOT_RETAINED = 0;
    
    /*
     * Pack the fields into a GSS auth identity.
     */
    virtual CFDictionaryRef getAttributes(void) CF_RETURNS_NOT_RETAINED = 0;
    
    /*
     * Called when the user selects the credential.
     */
    virtual void didBecomeSelected(Boolean *pbAutoLogin) = 0;
    virtual void didBecomeDeselected(void) = 0;
    virtual void didSubmit(void) = 0;

    virtual Boolean savePersisted(CFErrorRef *error) = 0;
    virtual Boolean deletePersisted(CFErrorRef *error) = 0;
};

class CUIProvider : public IUnknown {
public:
    /*
     * Initialize a new credential provider
     */
    virtual Boolean initWithController(CUIControllerRef controller, CFErrorRef *error) = 0;
    
    /*
     * Get all matching credentials for the selected attributes (attributes
     * may be NULL).
     */
    virtual CFArrayRef copyMatchingCredentials(CFDictionaryRef attributes,
                                               CUIUsageFlags usageFlags,
                                               CFIndex *defaultCredentialIndex,
                                               CFErrorRef *error) CF_RETURNS_RETAINED = 0;
};

class CUICredentialPersistence : public IUnknown {
public:
    /*
     * A provider can use this method to ask another provider to persist
     * a credential on its behalf. Persistence providers are responsible
     * for enumerating and updating existing persisted credentials; 
     * other concrete providers that wish to add a credential should call
     * CUIControllerCreatePersistenceForFactoryID() with the desired
     * persistence provider's factory ID.
     */
    virtual Boolean addCredentialWithAttributes(CFDictionaryRef attributes, CFErrorRef *error) = 0;

    /*
     * Persistence providers typically do not make a password available in
     * the credential dictionary until the user is ready to authenticate,
     * as a sensible security precaution. Concrete providers can use this
     * method to extract the password.
     */
    virtual CFTypeRef extractPassword(CFDictionaryRef attributes, CFErrorRef *error) = 0;
};
#else
typedef struct CUICredentialContext {
    IUNKNOWN_C_GUTS;
    CFStringRef (STDMETHODCALLTYPE *copyDescription)(void *thisPointer);
    CFArrayRef (STDMETHODCALLTYPE *getFields)(void *thisPointer);
    CFDictionaryRef (STDMETHODCALLTYPE *getAttributes)(void *thisPointer);
    void (STDMETHODCALLTYPE *didBecomeSelected)(void *thisPointer, Boolean *pbAutoLogin);
    void (STDMETHODCALLTYPE *didBecomeDeselected)(void *thisPointer);
    void (STDMETHODCALLTYPE *didSubmit)(void *thisPointer);
    Boolean (STDMETHODCALLTYPE *savePersisted)(void *thisPointer, CFErrorRef *error);
    Boolean (STDMETHODCALLTYPE *deletePersisted)(void *thisPointer, CFErrorRef *error);
} CUICredentialContext;

typedef struct CUIProvider {
    IUNKNOWN_C_GUTS;
    Boolean (STDMETHODCALLTYPE *initWithController)(void *thisPointer,
                                                    CUIControllerRef controller,
                                                    CFErrorRef *error);
    CFArrayRef (STDMETHODCALLTYPE *copyMatchingCredentials)(void *thisPointer,
                                                            CUIUsageFlags usageFlags,
                                                            CFDictionaryRef attributes,
                                                            CFIndex *defaultCredentialIndex,
                                                            CFErrorRef *error);
} CUIProvider;

typedef struct CUICredentialPersistence {
    IUNKNOWN_C_GUTS;
    Boolean (STDMETHODCALLTYPE *addCredentialWithAttributes)(void *thisPointer, CFDictionaryRef attributes, CFErrorRef *error);
    CFTypeRef (STDMETHODCALLTYPE *extractPassword)(void *thisPointer, CFDictionaryRef attributes, CFErrorRef *error);
} CUICredentialPersistence;
#endif /* defined(__cplusplus) */

#ifdef __cplusplus
extern "C" {
#endif

// F7356A4B-91A7-4455-AF3A-175D27449C9E
#define kCUIProviderInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xF7, 0x35, 0x6A, 0x4B, 0x91, 0xA7, 0x44, 0x55, 0xAF, 0x3A, 0x17, 0x5D, 0x27, 0x44, 0x9C, 0x9E)

// 2C8CF116-62BA-44B6-B894-C6A592E62566
#define kCUIProviderTypeID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x2C, 0x8C, 0xF1, 0x16, 0x62, 0xBA, 0x44, 0xB6, 0xB8, 0x94, 0xC6, 0xA5, 0x92, 0xE6, 0x25, 0x66)

// 20C3A840-6BC4-4B26-B705-43B4C46218E7
#define kCUICredentialInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x20, 0xC3, 0xA8, 0x40, 0x6B, 0xC4, 0x4B, 0x26, 0xB7, 0x05, 0x43, 0xB4, 0xC4, 0x62, 0x18, 0xE7)

// B4C5A7F1-1297-4AFE-BC26-533803B35389
#define kCUIPersistenceInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xB4, 0xC5, 0xA7, 0xF1, 0x12, 0x97, 0x4A, 0xFE, 0xBC, 0x26, 0x53, 0x38, 0x03, 0xB3, 0x53, 0x89)

CUI_EXPORT const CFStringRef kCUIAttrCredentialStatus;

// more information needed to make credential
CUI_EXPORT const CFStringRef kCUICredentialNotFinished;
// credential provider complete but no credential made
CUI_EXPORT const CFStringRef kCUICredentialFinished;
// credential made
CUI_EXPORT const CFStringRef kCUICredentialReturnCredentialFinished;
// no credential made but force caller to return
CUI_EXPORT const CFStringRef kCUICredentialReturnNoCredentialFinished;

CUI_EXPORT Boolean
_CUIControllerEnumerateCredentialsWithFlags(CUIControllerRef controller,
                                            CUIUsageFlags extraUsageFlags,
                                            CFDictionaryRef attributes,
                                            void (^cb)(CUICredentialRef, Boolean, CFErrorRef));

CUI_EXPORT CUIProvider *
CUIControllerFindProviderByFactoryID(CUIControllerRef controller, CFUUIDRef factoryID);

CUI_EXPORT CUICredentialPersistence *
CUIControllerCreatePersistenceForFactoryID(CUIControllerRef controller, CFUUIDRef factoryID);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
CUI_EXPORT CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknown *context);
}
#else
CUI_EXPORT CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknownVTbl *context);
#endif

/*
 * For PAM modules. This contains a CFDictionaryRef.
 */
#define CREDUI_ATTR_DATA    "CREDUI-ATTR-DATA"

#endif /* CredUICore_CUIProvider_h */
