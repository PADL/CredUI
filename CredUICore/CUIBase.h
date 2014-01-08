//
//  CUIBase.h
//  CredUI
//
//  Created by Luke Howard on 8/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CUIBase_h
#define CredUI_CUIBase_h

#include <CoreFoundation/CoreFoundation.h>
#include <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CUI_EXPORT CF_EXPORT

typedef CF_ENUM(CFIndex, CUIUsageScenario) {
    kCUIUsageScenarioInvalid = 0,
    kCUIUsageScenarioLogin,
    kCUIUsageScenarioNetwork
};

typedef CF_OPTIONS(CFIndex, CUIUsageFlags) {
    kCUIUsageFlagsGeneric               = 0x00000001,
    kCUIUsageFlagsMechanismOnly         = 0x00000010,
    kCUIUsageFlagsInCredOnly            = 0x00000020,
    kCUIUsageFlagsEnumerateAdmins       = 0x00000100,
    kCUIUsageFlagsEnumerateCurrentUser  = 0x00000200,
    kCUIUsageFlagsExcludeCertificates   = 0x04000000,
    kCUIUsageFlagsRequireCertificates   = 0x08000000,
    kCUIUsageFlagsPasswordOnlyOK        = 0x10000000,
    kCUIUsageFlagsKeepUsername          = 0x20000000,
    kCUIUsageFlagsExcludePersistedCreds = 0x40000000,
    kCUIUsageFlagsDoNotShowUI           = 0x80000000
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

#ifdef __cplusplus
}
#endif

#endif /* CredUI_CUIBase_h */
