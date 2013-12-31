//
//  CUIPromptForCredentials.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__PromptForCredentials__
#define __CredUI__PromptForCredentials__

#include <CoreFoundation/CoreFoundation.h>

/* CredUI flags, similar to Windows CredUI API */
typedef NS_OPTIONS(NSUInteger, CUIFlags) {
    CUIIncorrectPassword                = 0x00001,
    CUIDoNotPersist                     = 0x00002,
    CUIRequestAdministrator             = 0x00004,
    CUIExcludeCertificates              = 0x00008,
    CUIRequireCertificate               = 0x00010,
    CUIShowSaveCheckBox                 = 0x00040,
    CUIAlwaysShowUI                     = 0x00080,
    CUIRequireSmartcard                 = 0x00100,
    CUIPasswordOnlyOK                   = 0x00200,
    CUIValidateUsername                 = 0x00400,
    CUICompleteUsername                 = 0x00800,
    CUIPersist                          = 0x01000,
    CUIFlagsServerCredential            = 0x04000,
    CUIFlagsExpectConfirmation          = 0x20000,
    CUIFlagsGenericCredentials          = 0x40000,
    CUIFlagsUsernameTargetCredentials   = 0x80000,
    CUIFlagsKeepUsername                = 0x100000,
    CUIFlagsExcludePersistedCredentials = 0x10000000,
};

typedef struct __CUICredUIContext {
    CFIndex version;
    CFTypeRef parentWindow;
    CFStringRef messageText;
    CFStringRef titleText;
} CUICredUIContext;

Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        CFTypeRef reserved,
                        CFErrorRef authError,
                        CFStringRef username,
                        CFStringRef password,
                        CFDictionaryRef *outCredAttributes,
                        Boolean *save,
                        CUIFlags flags);


#endif