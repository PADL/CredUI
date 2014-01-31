//
//  CUIPromptForCredentials.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_PromptForCredentials_h
#define CredUI_PromptForCredentials_h

#include <CredUI/CUIBase.h>
#include <CredUICore/CUIBase.h>
#include <CredUICore/CUICredential.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Prompt the user for credentials.
 */
CUI_EXPORT Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        void **reserved,
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
CUI_EXPORT Boolean
CUIConfirmCredentials(CUICredentialRef credRef,
                      Boolean fSave,
                      CFErrorRef *error);

#ifdef __cplusplus
}
#endif

#endif
