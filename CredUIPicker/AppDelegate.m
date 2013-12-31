//
//  AppDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "AppDelegate.h"

#import <CredUI/CUIIdentityPicker.h>
#import <GSSKit/GSSKit.h>

#import <browserid.h>
#import <CFBrowserID.h>


@interface AppDelegate ()
@property (nonatomic, strong) CUIIdentityPicker *picker;
@end

@implementation AppDelegate

- (void)doGSSAPITests:(CUIIdentityPicker *)identityPicker
{
    GSSCredential *cred;
    NSError *error;
    
    NSLog(@"identityPicker selected GSS name = %@", identityPicker.selectedCredentialGSSName);
    
    cred = [[GSSCredential alloc] initWithName:[identityPicker selectedCredentialGSSName]
                                     mechanism:[GSSMechanism kerberosMechanism]
                                    attributes:[identityPicker selectedCredentialAttributes]
                                         error:&error];
    if (cred)
        NSLog(@"credential acquired: %@", cred);
    else if (error)
        NSLog(@"error: %@", error);
    else
        NSLog(@"no cred / no error");
    
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Picker did end: %@", [identityPicker selectedCredentialAttributes]);
    
    // OK, now let's try and do some GSS stuff
    [self doGSSAPITests:identityPicker];
}

- (IBAction)showIdentityPicker:(id)sender;
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials | CUIFlagsGSSAcquireCredsDisposition];
    
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