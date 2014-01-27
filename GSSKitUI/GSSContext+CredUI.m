//
//  GSSContext+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface GSSKitUIContext : NSObject
{
    NSError *_error;
    dispatch_semaphore_t _semaphore;
}
@property(nonatomic, retain) NSError *error;

- (void)signalCompletion;
- (void)waitForCompletion;
@end

@interface NSError (GSSKitErrorHelper)
+ (NSError *)GSSError:(OM_uint32)majorStatus
                     :(OM_uint32)minorStatus
                     :(GSSMechanism *)mech;
+ (NSError *)GSSError:(OM_uint32)majorStatus :(OM_uint32)minorStatus;
+ (NSError *)GSSError:(OM_uint32)majorStatus;

- (BOOL)_gssContinueNeeded;
- (BOOL)_gssError;
- (BOOL)_gssPromptingNeeded;
@end

@implementation GSSKitUIContext
@synthesize error = _error;

- init
{
    if ((self = [super init]) == nil)
        return nil;

    _semaphore = dispatch_semaphore_create(0);
    
    return self;
}

- (void)signalCompletion
{
    dispatch_semaphore_signal(_semaphore);
}

- (void)waitForCompletion
{
    dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
}

- (void)dealloc
{
    dispatch_release(_semaphore);
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

@end

@interface GSSContext ()
@property(nonatomic, readonly) gss_ctx_id_t _gssContext;
@end

@implementation GSSContext (CredUI)

static BOOL
_GSSNeedUpdateContextCredentialP(CUICredential *cuiCredential,
                                 GSSCredential *gssCredential)
{
    GSSName *cuiCredentialName = CFBridgingRelease([cuiCredential copyGSSName]);
    GSSName *gssCredentialName = gssCredential.name;
    
    return ![cuiCredentialName isEqualToName:gssCredentialName];
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    GSSKitUIContext *uiContext = (__bridge GSSKitUIContext *)contextInfo;
    CUICredential *credential = identityPicker.selectedCredential;
    GSSCredential *gssCred;
    NSError *error;
    
    if (returnCode != NSModalResponseOK) {
        uiContext.error = identityPicker.lastError ? identityPicker.lastError : [NSError GSSError:GSS_S_UNAVAILABLE];
        [uiContext signalCompletion];
        return;
    }
    
    self.mechanism = [GSSMechanism mechanismForCUICredential:credential];

    gssCred = [[GSSCredential alloc] initWithCUICredential:credential error:&error];
    if (gssCred == nil) {
        uiContext.error = error ? error : [NSError GSSError:GSS_S_NO_CRED];
        [uiContext signalCompletion];
        return;
    }
    
    self.targetName = identityPicker.targetName;
    
    /*
     * If the mechanism returned an error, then we need to create a new credential. If the
     * mechanism didn't, it's a little more subtle, because the credential provider may
     * just be updating the context with, for example, trust anchors. Only replace the
     * context credential if the selected credential name is different.
     */
    if ([self.lastError _gssError] ||
        _GSSNeedUpdateContextCredentialP(credential, self.credential))
        self.credential = gssCred;
    
#if !__has_feature(objc_arc)
    [gssCred release];
#endif

    [uiContext signalCompletion];
}

- (void)_runIdentityPicker:(id)uiContext
{
    CUIIdentityPicker *identityPicker;
    NSMutableDictionary *attributes = [@{
                                         (__bridge id)kCUIAttrNameType : (__bridge NSString *)kCUIAttrNameTypeGSSUsername,
                                         (__bridge id)kCUIAttrName : self.credential.name.displayString,
                                         (__bridge id)kCUIAttrSupportGSSCredential : @YES
                                         } mutableCopy];
   
    if (self.mechanism)
        [attributes setObject:self.mechanism.mechanismClass forKey:(__bridge id)kCUIAttrClass];
    
    identityPicker = [[CUIIdentityPicker alloc] initWithUsageScenario:kCUIUsageScenarioNetwork attributes:attributes];
    
    identityPicker.title = @"Identity Picker";
    identityPicker.targetName = self.targetName;
    identityPicker.context = self._gssContext;
    identityPicker.authError = self.lastError;
    
    [identityPicker runModalForWindow:self.window
                        modalDelegate:self
                       didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                          contextInfo:(__bridge void *)uiContext];
    
#if !__has_feature(objc_arc)
    [identityPicker autorelease];
    [attributes release];
#endif
}


- (BOOL)_promptForCredentials:(NSError **)error
{
    GSSKitUIContext *uiContext = [[GSSKitUIContext alloc] init];

    uiContext.error = nil;
    
    [self performSelectorOnMainThread:@selector(_runIdentityPicker:) withObject:uiContext waitUntilDone:TRUE];
    [uiContext waitForCompletion];

    if (error)
        *error = [uiContext error];

    return !uiContext.error;
}

- (id)initWithRequestFlags:(GSSFlags)flags
                     queue:(dispatch_queue_t)queue
            identityPicker:(CUIIdentityPicker *)identityPicker
{
    CUICredential *credential = identityPicker.selectedCredential;
    
    if (credential == nil)
        return nil;

    GSSCredential *gssCred = [[GSSCredential alloc] initWithCUICredential:credential error:NULL];

    self = [self initWithRequestFlags:flags queue:queue isInitiator:YES];
    
    self.mechanism = [GSSMechanism mechanismForCUICredential:credential];
    self.targetName = identityPicker.targetName;
    self.credential = gssCred;
    
#if !__has_feature(objc_arc)
    [gssCred release];
#endif
    
    return self;
}

@end
