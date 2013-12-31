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