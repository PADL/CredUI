//
//  CUIPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/GSSPromptForCredentials.h>

Boolean
__CUIPromptForCredentials(CFTypeRef targetName,
                          CFTypeRef gssContextHandle,
                          CUICredUIContext *uiContext,
                          CFErrorRef authError,
                          CFDictionaryRef inCredAttributes,
                          CFDictionaryRef *outCredAttributes,
                          Boolean *pfSave,
                          CUIFlags flags,
                          CUIAttributeClass attrClass)
{
    CUIIdentityPicker *identityPicker = [[CUIIdentityPicker alloc] initWithFlags:flags attributes:(__bridge NSDictionary *)inCredAttributes];
    CUICredential *selectedCredential;
    
    if (identityPicker == nil)
        return false;
    
    identityPicker.targetName = (__bridge id)targetName;
    identityPicker.GSSContextHandle = (__bridge GSSContext *)gssContextHandle;
    identityPicker.credUIContext = uiContext;
    identityPicker.authError = (__bridge NSError *)authError;
    identityPicker.persist = *pfSave;
 
    [identityPicker runModal];
    
    selectedCredential = identityPicker.selectedCredential;
    
    *outCredAttributes = CFBridgingRetain([selectedCredential attributesWithClass:attrClass]);
    
    *pfSave = identityPicker.persist;
    
    return [selectedCredential canSubmit];
}

CUI_EXPORT Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        CFTypeRef reserved,
                        CFErrorRef authError,
                        CFStringRef username,
                        CFStringRef password,
                        CFDictionaryRef *outCredAttributes,
                        Boolean *save,
                        CUIFlags flags)
{
    NSDictionary *inCredAttributes = @{
                                       (__bridge id)kCUIAttrNameType : (__bridge NSString *)kCUIAttrNameTypeGSSUsername,
                                       (__bridge id)kCUIAttrName : (__bridge NSString *)username,
                                       (__bridge id)kCUIAttrCredentialPassword : (__bridge NSString *)password
                                       };
    
    return __CUIPromptForCredentials(targetName,
                                     reserved,
                                     uiContext,
                                     authError,
                                     (__bridge CFDictionaryRef)inCredAttributes,
                                     outCredAttributes,
                                     save,
                                     flags,
                                     CUIAttributeClassGeneric);
}
