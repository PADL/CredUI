//
//  GSSPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

CUI_EXPORT Boolean
GSSPromptForCredentials(gss_name_t targetName,
                        CFTypeRef gssContextHandle,
                        CUICredUIContext *uiContext,
                        CFErrorRef authError,
                        CFDictionaryRef inCredAttributes,
                        CUICredentialRef *outCred,
                        Boolean *pfSave,
                        CUIFlags flags,
                        CFErrorRef *error)
{
    return _CUIPromptForCredentials(targetName, gssContextHandle, uiContext,
                                    authError, inCredAttributes, outCred,
                                    pfSave, flags, error);
}