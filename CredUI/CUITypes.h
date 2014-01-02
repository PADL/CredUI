//
//  CUITypes.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CUITypes_h
#define CredUI_CUITypes_h

/* CredUI flags, similar to Windows CredUI API */
typedef NS_OPTIONS(NSUInteger, CUIFlags) {
    CUIIncorrectPassword                = 0x00000001,
    CUIDoNotPersist                     = 0x00000002,
    CUIRequestAdministrator             = 0x00000004,
    CUIExcludeCertificates              = 0x00000008,
    CUIRequireCertificate               = 0x00000010,
    CUIShowSaveCheckBox                 = 0x00000040,
    CUIAlwaysShowUI                     = 0x00000080,
    CUIRequireSmartcard                 = 0x00000100,
    CUIPasswordOnlyOK                   = 0x00000200,
    CUIValidateUsername                 = 0x00000400,
    CUICompleteUsername                 = 0x00000800,
    CUIPersist                          = 0x00001000,
    CUIFlagsServerCredential            = 0x00004000,
    CUIFlagsExpectConfirmation          = 0x00020000,
    CUIFlagsGenericCredentials          = 0x00040000,
    CUIFlagsUsernameTargetCredentials   = 0x00080000,
    CUIFlagsKeepUsername                = 0x00100000,
    CUIFlagsExcludePersistedCredentials = 0x10000000,
};

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

#endif
