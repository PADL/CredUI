//
//  CUIBase.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CUIBase_h
#define CredUI_CUIBase_h

#include <CoreFoundation/CoreFoundation.h>

#ifndef CUI_EXPORT
#define CUI_EXPORT CF_EXPORT
#endif

/* CredUI flags, similar to Windows CredUI API */
typedef NS_OPTIONS(NSUInteger, CUIFlags) {
    CUIFlagsIncorrectPassword           = 0x00000001,
    CUIFlagsDoNotPersist                = 0x00000002,
    CUIFlagsRequestAdministrator        = 0x00000004,
    CUIFlagsExcludeCertificates         = 0x00000008,
    CUIFlagsRequireCertificate          = 0x00000010,
    CUIFlagsShowSaveCheckBox            = 0x00000040,
    CUIFlagsAlwaysShowUI                = 0x00000080,
    CUIFlagsRequireSmartcard            = 0x00000100,
    CUIFlagsPasswordOnlyOK              = 0x00000200,
    CUIFlagsValidateUsername            = 0x00000400,
    CUIFlagsCompleteUsername            = 0x00000800,
    CUIFlagsPersist                     = 0x00001000,
    CUIFlagsServerCredential            = 0x00004000,
    CUIFlagsExpectConfirmation          = 0x00020000,
    CUIFlagsGenericCredentials          = 0x00040000,
    CUIFlagsUsernameTargetCredentials   = 0x00080000,
    CUIFlagsKeepUsername                = 0x00100000,
    CUIFlagsExcludePersistedCredentials = 0x10000000,
};

/* CredUI context, used for C APIs */
typedef struct __CUICredUIContext {
    CFIndex version;
    CFTypeRef parentWindow;
    CFStringRef messageText;
    CFStringRef titleText;
} CUICredUIContext;

typedef NS_ENUM(NSUInteger, CUIAttributeClass) {
    CUIAttributeClassGeneric                = 1,    // generic credential attributes
    CUIAttributeClassGSSInitialCred         = 2,    // attributes for gss_aapl_initial_cred
    CUIAttributeClassGSSItem                = 4     // attributes for GSSItemAdd
};

#endif /* CredUI_CUIBase_h */
