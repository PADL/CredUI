//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUICredential__
#define __CredUI__CUICredential__

#ifdef __cplusplus
extern "C" {
#endif

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
    
typedef CF_ENUM(CFIndex, CUIUsageScenario) {
    kCUIUsageScenarioInvalid = 0,
    kCUIUsageScenarioLogin,
    kCUIUsageScenarioNetwork
};
    
typedef CF_OPTIONS(CFIndex, CUIUsageFlags) {
    kCUIUsageFlagsGeneric = 0x00000001,
    kCUIUsageFlagsSaveCheckbox = 0x00000002,
    kCUIUsageFlagsMechanismOnly = 0x00000010,
    kCUIUsageFlagsInCredOnly = 0x00000020,
    kCUIUsageFlagsEnumerateAdmins = 0x00000100,
    kCUIUsageFlagsEnumerateCurrentUser = 0x00000200
};

typedef struct __CUICredential *CUICredentialRef;

extern CFTypeID
CUICredentialGetTypeID(void);

#ifdef __cplusplus
class CUICredentialContext : public IUnknown {
public:
    virtual CFStringRef copyDescription(void);
    virtual CFArrayRef getFields(void); // array of CUIFieldRef
};
#else
typedef struct CUICredentialContext {
    IUNKNOWN_C_GUTS;
    CFStringRef (STDMETHODCALLTYPE *copyDescription)(void *thisPointer);
    CFArrayRef (STDMETHODCALLTYPE *getFields)(void *thisPointer);
} CUICredentialContext;

#endif /* defined(__cplusplus) */
    
extern CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, CUICredentialContext *context);
    
extern CFArrayRef
CUICredentialGetFields(CUICredentialRef cred);
    
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUICredential__) */