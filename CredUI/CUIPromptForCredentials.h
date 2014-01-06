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
#include <CredUICore/CredUICore.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Prompt the user for credentials.
 */
Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        CFTypeRef reserved,
                        CFErrorRef authError,
                        CFStringRef username,
                        CFStringRef password,
                        CUICredentialRef *outCred,
                        Boolean *fSave,
                        CUIFlags flags,
                        CFErrorRef *error);

/*
 * If CUIFlagsExpectConfirmation was passed to CUIPromptForCredentials(),
 * then call this function after the credentials have been successfully used.
 */
Boolean
CUIConfirmCredentials(CUICredentialRef credRef,
                      Boolean fSave,
                      CFErrorRef *error);

#ifdef __cplusplus
}
#endif

#endif
