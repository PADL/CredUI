//
//  CredUIPickerDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/GSSPromptForCredentials.h>
#import <CredUICore/CUIAttributes.h>

#import <GSSKit/GSSKit.h>

#import "CredUIPickerDelegate.h"
#import "GSSCredential+CredUI.h"
#import "GSSItem+CredUI.h"
#import "GSSMechanism+CredUI.h"

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

- (NSError *)doInitAcceptGSSContext:(CUIIdentityPicker *)identityPicker
                     initiatorCred:(GSSCredential *)initiatorCred
{
    GSSContext *contextHandle;
    NSError *err = [self doInitAcceptGSSContext:identityPicker.targetName
                                  initiatorCred:initiatorCred
                                      mechanism:[GSSMechanism mechanismForCUICredential:identityPicker.selectedCredential]
                                  contextHandle:&contextHandle];
    return err;
}

- (NSError *)doInitAcceptGSSContext:(id)targetName
                     initiatorCred:(GSSCredential *)initiatorCred
                         mechanism:(GSSMechanism *)mechanism
                      contextHandle:(GSSContext **)contextHandle
{
    dispatch_queue_t queue;
    GSSContext *initiatorCtx = nil, *acceptorCtx = nil;
    NSError *err = nil;
    __block NSData *initiatorToken = nil, *acceptorToken = nil;
    
    queue = dispatch_queue_create("com.padl.CredUIPicker.queue", DISPATCH_QUEUE_SERIAL);
    
    initiatorCtx = [[GSSContext alloc] initWithRequestFlags:0 //GSS_C_MUTUAL_FLAG
                                                      queue:queue
                                                isInitiator:YES];
    
    initiatorCtx.mechanism = mechanism;
    initiatorCtx.targetName = targetName;
    initiatorCtx.credential = initiatorCred;
    
    acceptorCtx = [[GSSContext alloc] initWithRequestFlags:0
                                                     queue:queue
                                               isInitiator:NO];
    
    acceptorCtx.credential = [GSSCredential credentialWithName:targetName
                                                     mechanism:initiatorCtx.mechanism
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

    *contextHandle = initiatorCtx;
    return [initiatorCtx lastError];
}

- (void)GSSIC_identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"IC picker did end: %@", identityPicker.selectedCredential.attributes);
    
    // OK, now let's try and do some GSS stuff
    GSSCredential *cred = [self acquireGSSCred:identityPicker];
    
    if (cred)
        (void) [self doInitAcceptGSSContext:identityPicker initiatorCred:cred];
}

- (IBAction)showIdentityPickerGSSIC:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials];
    
    self.picker.title = @"Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(GSSIC_identityPickerDidEnd:returnCode:contextInfo:)
                       contextInfo:NULL];
}

- (IBAction)gssAuthWithIdentityPickerFallback:(id)sender
{
    GSSName *targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    GSSMechanism *mechanism = [GSSMechanism personaMechanism];
    GSSCredential *cred = [GSSCredential credentialWithName:@"lukeh@padl.com" mechanism:mechanism];
    NSError *err;
    GSSContext *contextHandle = nil;
    
    err = [self doInitAcceptGSSContext:targetName initiatorCred:cred mechanism:mechanism contextHandle:&contextHandle];
    if (err && GSSIsPromptingNeeded((__bridge CFErrorRef)err)) {
        NSDictionary *attributes = @{
                                     (__bridge NSString *)kCUIAttrClass: [mechanism name],
                                     (__bridge NSString *)kCUIAttrName: [[cred name] description]
                                     };
        
        self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials attributes:attributes];
        
        self.picker.GSSContextHandle = contextHandle;
        self.picker.title = @"Identity Picker";
        self.picker.message = @"Choose an identity";
        self.picker.targetName = targetName;
        
        [self.picker runModalForWindow:self.window
                         modalDelegate:self
                        didEndSelector:@selector(GSSIC_identityPickerDidEnd:returnCode:contextInfo:)
                           contextInfo:NULL];

        err = [self doInitAcceptGSSContext:targetName initiatorCred:cred mechanism:mechanism contextHandle:&contextHandle];
    }
    if (err)
        NSLog(@"failed with code: %@", err);
}

- (void)GSSItem_identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Item picker did end: %@", identityPicker.selectedCredential.attributes);
    NSError *error = nil;
    
    // OK, now let's try and do some GSS stuff
    NSArray *items = [GSSItem itemsMatchingCUICredential:identityPicker.selectedCredential error:&error];
    
    if (items)
        NSLog(@"Item --> %@", items);
    else
        NSLog(@"Item error --> %@", error);
}

- (IBAction)showIdentityPickerGSSItem:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:0];
    
    self.picker.title = @"Item Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"http://www.padl.com"];

    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(GSSItem_identityPickerDidEnd:returnCode:contextInfo:)
                       contextInfo:NULL];

}


- (void)Generic_identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Generic picker did end: %@", identityPicker.selectedCredential.attributes);
}

- (IBAction)showIdentityPickerGeneric:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsGenericCredentials | CUIFlagsAlwaysShowUI];
    
    self.picker.title = @"Generic Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"https://www.padl.com"];
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(Generic_identityPickerDidEnd:returnCode:contextInfo:)
                       contextInfo:NULL];
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

@end
