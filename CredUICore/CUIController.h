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
    kCUIUsageFlagsExcludePersistedCreds = 0x40000000,
    kCUIUsageFlagsNoUI                  = 0x80000000
};

typedef CF_OPTIONS(CFIndex, CUICredUIContextProperties) {
    kCUICredUIContextPropertyParentWindow   = 0x1,
    kCUICredUIContextPropertyMessageText    = 0x2,
    kCUICredUIContextPropertyTitleText      = 0x4,
    kCUICredUIContextPropertyAll            = 0xf
};
    
#ifndef CredUI_CUITypes_h
typedef struct __CUICredUIContext {
    CFIndex version;
    CFTypeRef parentWindow;
    CFStringRef messageText;
    CFStringRef titleText;
} CUICredUIContext;
#endif

typedef struct __CUIController *CUIControllerRef;

CFTypeID
CUIControllerGetTypeID(void);

CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usage,
                    CUIUsageFlags usageFlags);
    
void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef authIdentity);

CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller);

void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError);

CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller);

Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller,
                              CUICredUIContextProperties whichProperties,
                              const CUICredUIContext *context);

const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller);

void
CUIControllerSetGssContextHandle(CUIControllerRef controller, CFTypeRef ctx);

CFTypeRef
CUIControllerGetGssContextHandle(CUIControllerRef controller);

void
CUIControllerSetGssTargetName(CUIControllerRef controller, gss_name_t target);

gss_name_t
CUIControllerGetGssTargetName(CUIControllerRef controller);

Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef, CFErrorRef));
    
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIController__) */