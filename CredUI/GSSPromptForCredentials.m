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
    CUIIdentityPicker *identityPicker = [[CUIIdentityPicker alloc] initWithFlags:flags attributes:(__bridge NSDictionary *)inCredAttributes];
    CUICredential *selectedCredential;
    
    if (identityPicker == nil)
        return false;
    
    identityPicker.GSSContextHandle = (__bridge GSSContext *)gssContextHandle;
    identityPicker.authError = (__bridge NSError *)authError;
    identityPicker.saveToKeychain = *pfSave;
    
    [identityPicker runModal];
    
    selectedCredential = identityPicker.selectedCredential;
    
    *outCredAttributes = CFBridgingRetain(selectedCredential.attributes);
    
    *pfSave = identityPicker.saveToKeychain;
    
    return [identityPicker _canReturnWithCredential:selectedCredential];
}