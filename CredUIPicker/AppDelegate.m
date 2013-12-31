//
//  AppDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "AppDelegate.h"

#import <CredUI/CUIIdentityPicker.h>

@interface AppDelegate ()
@property (nonatomic, strong) CUIIdentityPicker *picker;
@end

@implementation AppDelegate

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Picker did end: %@", [identityPicker selectedCredentialAttributes]);
}

- (IBAction)showIdentityPicker:(id)sender;
{
    self.picker = [[CUIIdentityPicker alloc] init];
    
    self.picker.title = @"Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = @"host/browserid.padl.com";
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                       contextInfo:NULL];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

@end
