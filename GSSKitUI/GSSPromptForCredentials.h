//
//  GSSPromptForCredentials.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_GSSPromptForCredentials_h
#define CredUI_GSSPromptForCredentials_h

#include <GSS/GSS.h>
#include <CredUI/CUIBase.h>
#include <CredUICore/CUICredential.h>

#ifdef __cplusplus
extern "C" {
#endif

CUI_EXPORT Boolean
GSSPromptForCredentials(gss_const_name_t targetName,
                        const gss_ctx_id_t gssContextHandle,
                        CUICredUIContext *uiContext,
                        CFErrorRef authError,
                        CFDictionaryRef inCredAttributes,
                        CUICredentialRef *outCred,
                        Boolean *pfSave,
                        CUIFlags flags,
                        CFErrorRef *error);

#ifdef __cplusplus
}
#endif

#endif
