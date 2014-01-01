//
//  CredUIPickerDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <GSSKit/GSSKit.h>

#import "CredUIPickerDelegate.h"
#import "GSSCredential+CredUI.h"

@interface CredUIPickerDelegate ()
@property (nonatomic, strong) CUIIdentityPicker *picker;
@end


@implementation CredUIPickerDelegate

- (GSSCredential *)acquireGSSCred:(CUIIdentityPicker *)identityPicker
{
    GSSCredential *cred;
    NSError *error;
    
    if (identityPicker.selectedCredential == nil) {
        NSLog(@"no selected credential");
        return nil;
    }

    cred = [[GSSCredential alloc] initWithCUICredential:identityPicker.selectedCredential error:&error];
    if (cred)
        NSLog(@"credential acquired: %@", cred);
    else if (error)
        NSLog(@"error: %@", error);
    else
        NSLog(@"no cred / no error");
    
    return cred;
}

- (NSUInteger)initAcceptGSSContext:(CUIIdentityPicker *)identityPicker
                     initiatorCred:(GSSCredential *)initiatorCred
{
    dispatch_queue_t queue;
    GSSContext *initiatorCtx = nil, *acceptorCtx = nil;
    NSError *err = nil;
    __block NSData *initiatorToken = nil, *acceptorToken = nil;
    
    queue = dispatch_queue_create("com.padl.CredUIPicker.queue", DISPATCH_QUEUE_SERIAL);
    
    initiatorCtx = [[GSSContext alloc] initWithRequestFlags:0 //GSS_C_MUTUAL_FLAG
                                                      queue:queue
                                                isInitiator:YES];
    
    initiatorCtx.targetName = identityPicker.targetName;
    initiatorCtx.credential = initiatorCred;
    
    acceptorCtx = [[GSSContext alloc] initWithRequestFlags:0
                                                     queue:queue
                                               isInitiator:NO];
    
    acceptorCtx.credential = [GSSCredential credentialWithName:identityPicker.targetName
                                                     mechanism:[GSSMechanism personaMechanism]
                                                    attributes:@{GSSCredentialUsage : GSSCredentialUsageAccept}
                                                         error:&err];
    
    do {
        dispatch_semaphore_t sema = dispatch_semaphore_create(0);
        
        [initiatorCtx stepWithData:acceptorToken
                 completionHandler:^(NSData *outputToken, NSError *error) {
                     initiatorToken = outputToken;
                     dispatch_semaphore_signal(sema);
                 }];
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        
        if ([initiatorCtx didError] || ![initiatorToken length])
            break;
        
        NSLog(@"Sending initiator token %@", initiatorToken);
        
        [acceptorCtx stepWithData:initiatorToken
                completionHandler:^(NSData *outputToken, NSError *error) {
                    acceptorToken = outputToken;
                    dispatch_semaphore_signal(sema);
                }];
        dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
        if ([acceptorCtx didError])
            break;
        
        NSLog(@"Sending acceptor token %@", acceptorToken);
    } while ([initiatorCtx isContinueNeeded]);
    
    NSLog(@"Initiator status %ld acceptor status %ld", [[initiatorCtx lastError] code], [[acceptorCtx lastError] code]);

    return [[initiatorCtx lastError] code];
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Picker did end: %@", identityPicker.selectedCredential.attributes);
    
    // OK, now let's try and do some GSS stuff
    GSSCredential *cred = [self acquireGSSCred:identityPicker];
    
    if (cred)
        (void) [self initAcceptGSSContext:identityPicker initiatorCred:cred];
}

- (IBAction)showIdentityPicker:(id)sender;
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials];

    self.picker.title = @"Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = @"host@browserid.padl.com";
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                       contextInfo:NULL];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

@end
