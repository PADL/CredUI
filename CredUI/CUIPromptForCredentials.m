//
//  CUIPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/GSSPromptForCredentials.h>

Boolean
_CUIPromptForCredentials(CFTypeRef targetName,
                         const void *context,
                         CUICredUIContext *uiContext,
                         CFErrorRef authError,
                         CFDictionaryRef inCredAttributes,
                         CUICredentialRef *outCred,
                         Boolean *pfSave,
                         CUIFlags flags,
                         CFErrorRef *error)
{
    CUIIdentityPicker *identityPicker = [[CUIIdentityPicker alloc] initWithFlags:flags attributes:(__bridge NSDictionary *)inCredAttributes];
    CUICredential *selectedCredential;
    NSModalSession modalSession;
    NSModalResponse modalResponse;
    
    if (error != NULL)
        *error = NULL;
    if (identityPicker == nil)
        return false;
    
    identityPicker.targetName = (__bridge id)targetName;
    identityPicker.context = context;
    identityPicker.authError = (__bridge NSError *)authError;
    identityPicker.persist = *pfSave;
 
    /* Don't clobber the internal parent window, which is the window the CredUI window itself */
    [identityPicker->_reserved[0] setCredUIContext:uiContext properties:kCUICredUIContextPropertyAll & ~(kCUICredUIContextPropertyParentWindow)];

    if (uiContext && uiContext->parentWindow) {
        [identityPicker runModalForWindow:(__bridge NSWindow *)uiContext->parentWindow
                            modalDelegate:nil
                           didEndSelector:NULL
                              contextInfo:NULL];
    }
    
    modalSession = [NSApp beginModalSessionForWindow:[identityPicker->_reserved[0] identityPickerPanel]];
    do {
        modalResponse = [NSApp runModalSession:modalSession];
    } while (modalResponse == NSModalResponseContinue &&
             [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate distantFuture]]);
    
    [NSApp endModalSession:modalSession];
    
    selectedCredential = identityPicker.selectedCredential;
    
    *outCred = (CUICredentialRef)CFBridgingRetain(selectedCredential);
    
    *pfSave = identityPicker.persist;
    if (error)
        *error = (CFErrorRef)CFBridgingRetain(identityPicker.lastError);
    
#if !__has_feature(objc_arc)
    [identityPicker release];
#endif
    
    return [selectedCredential canSubmit];
}

CUI_EXPORT Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        const void *reserved,
                        CFErrorRef authError,
                        CFStringRef username,
                        CFStringRef password,
                        CUICredentialRef *outCred,
                        Boolean *pfSave,
                        CUIFlags flags,
                        CFErrorRef *error)
{
    NSDictionary *inCredAttributes = @{
                                       (__bridge id)kCUIAttrNameType : (__bridge NSString *)kCUIAttrNameTypeGSSUsername,
                                       (__bridge id)kCUIAttrName : (__bridge NSString *)username,
                                       (__bridge id)kCUIAttrCredentialPassword : (__bridge NSString *)password
                                       };
    
    return _CUIPromptForCredentials(targetName,
                                    reserved,
                                    uiContext,
                                    authError,
                                    (__bridge CFDictionaryRef)inCredAttributes,
                                    outCred,
                                    pfSave,
                                    flags,
                                    error);
}

CUI_EXPORT Boolean
CUIConfirmCredentials(CUICredentialRef credRef,
                      Boolean fSave,
                      CFErrorRef *error)
{
    CUICredential *credential = (__bridge CUICredential *)credRef;
    Boolean ret = true;
    NSError *nsError = nil;

    if (error)
        *error = NULL;
    
    if (fSave) {
        ret = [credential savePersisted:&nsError];
        if (error)
            *error = (CFErrorRef)CFBridgingRetain(nsError);
    }
    
    return ret;
}
