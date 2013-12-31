//
//  GSSPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

Boolean
GSSPromptForCredentials(gss_name_t targetName,
                        CFTypeRef gssContextHandle,
                        CUICredUIContext *uiContext,
                        CFErrorRef authError,
                        CFDictionaryRef inCredAttributes,
                        CFDictionaryRef *outCredAttributes,
                        Boolean *pfSave,
                        CUIFlags flags)
{
    return __CUIPromptForCredentials(targetName, gssContextHandle, uiContext,
                                     authError, inCredAttributes, outCredAttributes,
                                     pfSave, flags, CUIAttributeClassGSSInitialCred);
}

#ifndef GSS_S_PROMPTING_NEEDED
#define GSS_S_PROMPTING_NEEDED (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 5))
#endif

Boolean
GSSIsPromptingNeeded(CFErrorRef authError)
{
    NSError *error = (__bridge NSError *)authError;
    OM_uint32 major = [error.userInfo[@"kGSSMajorErrorCode"] unsignedIntValue];

    return (major == GSS_S_NO_CRED || major == GSS_S_PROMPTING_NEEDED);
}