//
//  CUIPromptForCredentials.m
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIPromptForCredentialsContextBox : NSObject <CUIContextBoxing>
{
@public
    void **_context;
}
@end

@implementation CUIPromptForCredentialsContextBox
- (void)setContext:(void *)context
{
    *_context = context;
}

- (void *)context
{
    return *_context;
}

- (NSData *)exportContext
{
    NSData *data;
    OM_uint32 major, minor;
    gss_buffer_desc exportedContext = GSS_C_EMPTY_BUFFER;
    gss_ctx_id_t context = self.context;
    
    major = gss_export_sec_context(&minor, &context, &exportedContext);
    if (GSS_ERROR(major))
        return nil;
    
    data = [NSData dataWithBytes:exportedContext.value length:exportedContext.length];
    gss_release_buffer(&minor, &exportedContext);
    
    self.context = GSS_C_NO_CONTEXT;
    
    return data;
}

- (BOOL)importContext:(NSData *)data
{
    OM_uint32 major, minor;
    gss_buffer_desc exportedContext;
    void *context = GSS_C_NO_CONTEXT;
    
    if (!data.length)
        return NO;
    
    exportedContext.length = data.length;
    exportedContext.value = (void *)data.bytes;
    
    major = gss_import_sec_context(&minor, &exportedContext, (gss_ctx_id_t *)&context);
    if (GSS_ERROR(major))
        return NO;
    
    self.context = context;
    
    return YES;
}

@end

CUI_EXPORT Boolean
_CUIPromptForCredentials(CFTypeRef targetName,
                         void **context,
                         CUICredUIContext *uiContext,
                         CFErrorRef authError,
                         CFDictionaryRef inCredAttributes,
                         CUICredentialRef *outCred,
                         Boolean *pfSave,
                         CUIFlags flags,
                         CFErrorRef *error)
{
    CUIIdentityPicker *identityPicker = [[CUIIdentityPicker alloc] initWithUsageScenario:kCUIUsageScenarioNetwork
                                                                              attributes:(__bridge NSDictionary *)inCredAttributes
                                                                                   flags:flags];
    CUIPromptForCredentialsContextBox *contextBox = [[CUIPromptForCredentialsContextBox alloc] init];
    CUICredential *selectedCredential;
    NSModalSession modalSession;
    NSModalResponse modalResponse;
    
    if (error != NULL)
        *error = NULL;
    if (identityPicker == nil)
        return false;
   
    contextBox->_context = context;
 
    identityPicker.targetName = (__bridge id)targetName;
    identityPicker.contextBox = contextBox;
    identityPicker.authError = (__bridge NSError *)authError;
    identityPicker.persist = *pfSave;
 
    /* Don't clobber the internal parent window, which is the window the CredUI window itself */
    [identityPicker->_reserved[0] setCredUIContext:uiContext properties:kCUICredUIContextPropertyAll & ~(kCUICredUIContextPropertyParentWindow)];

    if (uiContext && uiContext->parentWindow) {
        [identityPicker beginSheetModalForWindow:(__bridge NSWindow *)uiContext->parentWindow
                                   modalDelegate:nil
                                  didEndSelector:NULL
                                     contextInfo:NULL];
    }
    
    modalSession = [NSApp beginModalSessionForWindow:[identityPicker->_reserved[0] window]];
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
    [contextBox release];
    [identityPicker release];
#endif
    
    return [selectedCredential canSubmit];
}

CUI_EXPORT Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        void **reserved,
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
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
    __block Boolean ret = true;
    
    if (error)
        *error = NULL;
    
    if (fSave) {
        [credential savePersisted:^(NSError *nsError) {
            *error = (CFErrorRef)CFBridgingRetain(nsError);
            ret = !!nsError;
            dispatch_semaphore_signal(semaphore);
        }];
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
    }
    
#if !__has_feature(objc_arc)
    dispatch_release(semaphore);
#endif
    
    return ret;
}
