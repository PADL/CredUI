//
//  CUIBase.h
//  CredUI
//
//  Created by Luke Howard on 8/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUIBase_h
#define CredUICore_CUIBase_h

#include <CoreFoundation/CoreFoundation.h>
#include <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CUI_EXPORT
#define CUI_EXPORT CF_EXPORT
#endif
    
typedef CF_ENUM(CFIndex, CUIUsageScenario) {
    kCUIUsageScenarioInvalid = 0,
    kCUIUsageScenarioLogin,                             /* loginwindow, unimplemented */
    kCUIUsageScenarioNetwork                            /* GSS or other network caller */
};

typedef CF_OPTIONS(CFIndex, CUIUsageFlags) {
    kCUIUsageFlagsGeneric               = 0x00000001,   /* generic (non-GSS) credentials */
    kCUIUsageFlagsInClassOnly           = 0x00000010,   /* only enumerate providers that support this class */
    kCUIUsageFlagsInCredOnly            = 0x00000020,   /* only enumerate credentials in controller attributes */
    kCUIUsageFlagsEnumerateAdmins       = 0x00000100,   /* unimplemented, login scenario only */
    kCUIUsageFlagsEnumerateCurrentUser  = 0x00000200,   /* unimplemented, login scenario only */
    kCUIUsageFlagsExcludeCertificates   = 0x04000000,   /* do not enumerate certificate credentials */
    kCUIUsageFlagsRequireCertificates   = 0x08000000,   /* only enumerate certificate credentials */
    kCUIUsageFlagsPasswordOnlyOK        = 0x10000000,   /* do not display username field */
    kCUIUsageFlagsKeepUsername          = 0x20000000,   /* username field is readonly */
    kCUIUsageFlagsExcludePersistedCreds = 0x40000000,   /* do not enumerate persisted credentials */
    kCUIUsageFlagsDoNotShowUI           = 0x80000000    /* caller is a console app */
};

typedef CF_OPTIONS(CFIndex, CUICredUIContextProperties) {
    kCUICredUIContextPropertyParentWindow   = 0x1,
    kCUICredUIContextPropertyMessageText    = 0x2,
    kCUICredUIContextPropertyTitleText      = 0x4,
    kCUICredUIContextPropertyAll            = 0xf
};

#ifndef CredUI_CUIBase_h
typedef struct __CUICredUIContext {
    /*
     * Version of structure, must be zero.
     */
    CFIndex version;
    /*
     * When calling GSSPromptForCredentials/CUIPromptForCredentials, this is
     * the application's window (if the application wants to run CredUI as a
     * sheet inside it).
     *
     * To the provider, this is the window containing the CredUI.
     *
     * Either way, it is safe to cast to a NSWindow.
     */
    CFTypeRef parentWindow;
    /*
     * CredUI message text.
     */
    CFStringRef messageText;
    /*
     * CredUI title text.
     */
    CFStringRef titleText;
} CUICredUIContext;
#endif

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIBase_h */
