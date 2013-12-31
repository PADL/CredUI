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
    if ((flags & (CUIFlagsGSSAcquireCredsDisposition | CUIFlagsGSSItemDisposition)) == 0)
        flags |= CUIFlagsGSSAcquireCredsDisposition;
    
    return __CUIPromptForCredentials(targetName, gssContextHandle, uiContext,
                                     authError, inCredAttributes, outCredAttributes,
                                     pfSave, flags);
}