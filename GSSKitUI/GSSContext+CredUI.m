//
//  GSSContext+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUICore/CredUICore.h>
#import "GSSContext+CredUI.h"

@interface GSSKitUI_ErrorContainer : NSObject
@property(nonatomic, retain) NSError *error;
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

@implementation GSSKitUI_ErrorContainer
@end

@implementation GSSContext (CredUI)

BOOL _GSSNeedUpdateContextCredentialP(CUICredential *cuiCredential,
                                      GSSCredential *gssCredential)
{
    GSSName *cuiCredentialName = CFBridgingRelease([cuiCredential copyGSSName]);
    GSSName *gssCredentialName = gssCredential.name;
    
    return ![cuiCredentialName isEqualToName:gssCredentialName];
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    GSSKitUI_ErrorContainer *errorContainer = (__bridge GSSKitUI_ErrorContainer *)contextInfo;
    CUICredential *credential = identityPicker.selectedCredential;
    GSSCredential *gssCred;
    NSError *error;
    
    if (returnCode != NSModalResponseStop) {
        errorContainer.error = identityPicker.lastError ? identityPicker.lastError : [NSError GSSError:GSS_S_UNAVAILABLE];
        return;
    }
    
    self.mechanism = [GSSMechanism mechanismForCUICredential:credential];

    gssCred = [[GSSCredential alloc] initWithCUICredential:credential error:&error];
    if (gssCred == nil) {
        errorContainer.error = error ? error : [NSError GSSError:GSS_S_NO_CRED];
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
}

- (void)_runIdentityPicker:(id)errorContainer
{
    CUIIdentityPicker *identityPicker;
    NSMutableDictionary *attributes = [@{
                                         (__bridge id)kCUIAttrNameType : (__bridge NSString *)kCUIAttrNameTypeGSSUsername,
                                         (__bridge id)kCUIAttrName : self.credential.name.displayString,
                                         (__bridge id)kCUIAttrSupportGSSCredential : (__bridge id)kCFBooleanTrue
                                         } mutableCopy];
   
    if (self.mechanism)
        attributes[(__bridge id)kCUIAttrClass] = self.mechanism.mechanismClass;
    
    identityPicker = [[CUIIdentityPicker alloc] initWithFlags:0 attributes:attributes];
    
    identityPicker.title = @"Identity Picker";
    identityPicker.targetName = self.targetName;
    identityPicker.GSSContextHandle = self;
    identityPicker.authError = self.lastError;
    
    [identityPicker runModalForWindow:self.window
                        modalDelegate:self
                       didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                          contextInfo:(__bridge void *)errorContainer];
}


- (BOOL)_promptForCredentials:(NSError **)error
{
    GSSKitUI_ErrorContainer *errorContainer = [[GSSKitUI_ErrorContainer alloc] init];

    if (error)
        *error = [errorContainer error];
    
    errorContainer.error = nil;
    
    [self performSelectorOnMainThread:@selector(_runIdentityPicker:) withObject:errorContainer waitUntilDone:TRUE];
    
    return !errorContainer.error;
}

- (id)initWithRequestFlags:(GSSFlags)flags
                     queue:(dispatch_queue_t)queue
            identityPicker:(CUIIdentityPicker *)identityPicker
{
    CUICredential *credential = identityPicker.selectedCredential;
    
    if (credential == nil)
        return nil;
    
    self = [self initWithRequestFlags:flags queue:queue isInitiator:YES];
    
    self.mechanism = [GSSMechanism mechanismForCUICredential:credential];
    self.targetName = identityPicker.targetName;
    self.credential = [[GSSCredential alloc] initWithCUICredential:credential error:NULL];
    
    return self;
}

@end
