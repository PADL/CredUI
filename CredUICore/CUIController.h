//
//  CUIController.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIController__
#define __CredUI__CUIController__

#include <CoreFoundation/CoreFoundation.h>
#include <GSS/GSS.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef CF_ENUM(CFIndex, CUIUsageScenario) {
    kCUIUsageScenarioInvalid = 0,
    kCUIUsageScenarioLogin,
    kCUIUsageScenarioNetwork
};

typedef CF_OPTIONS(CFIndex, CUIUsageFlags) {
    kCUIUsageFlagsGeneric               = 0x00000001,
    kCUIUsageFlagsSaveCheckbox          = 0x00000002,
    kCUIUsageFlagsMechanismOnly         = 0x00000010,
    kCUIUsageFlagsInCredOnly            = 0x00000020,
    kCUIUsageFlagsEnumerateAdmins       = 0x00000100,
    kCUIUsageFlagsEnumerateCurrentUser  = 0x00000200,
    kCUIUsageFlagsNoUI                  = 0x80000000
};

typedef struct __CUIContext {
    CFIndex version;
    CFTypeRef parentWindow;
    CFStringRef messageText;
    CFStringRef titleText;
} CUICredUIContext;

typedef struct __CUIController *CUIControllerRef;

CFTypeID
CUIControllerGetTypeID(void);

CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usage,
                    CUIUsageFlags usageFlags);
    
void
CUIControllerSetAuthIdentity(CUIControllerRef controller, CFDictionaryRef authIdentity);

CFDictionaryRef
CUIControllerGetAuthIdentity(CUIControllerRef controller);

void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError);

CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller);

Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller, const CUICredUIContext *context);

const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller);

void
CUIControllerSetSaveToKeychain(CUIControllerRef controller, Boolean save);

Boolean
CUIControllerGetSaveToKeychain(CUIControllerRef controller);

void
CUIControllerSetGssContextHandle(CUIControllerRef controller, CFTypeRef ctx);

CFTypeRef
CUIControllerGetGssContextHandle(CUIControllerRef controller);

void
CUIControllerSetGssTargetName(CUIControllerRef controller, gss_name_t target);

gss_name_t
CUIControllerGetGssTargetName(CUIControllerRef controller);

Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef));
    
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIController__) */