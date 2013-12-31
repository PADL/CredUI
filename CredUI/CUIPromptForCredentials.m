//
//  CUIPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/GSSPromptForCredentials.h>

Boolean
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
    CFErrorRef error = NULL;
    gss_name_t target = GSSCreateName(targetName, GSS_C_NT_HOSTBASED_SERVICE, &error);
    
    if (target == NULL) {
        if (error)
            CFRelease(error);
        return false;
    }
    
    return GSSPromptForCredentials(target,
                                   reserved,
                                   uiContext,
                                   authError,
                                   (__bridge CFDictionaryRef)inCredAttributes,
                                   outCredAttributes,
                                   save,
                                   flags);
}
