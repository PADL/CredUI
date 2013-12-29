//
//  AppDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "AppDelegate.h"

#import <CredUI/CUIIdentityPicker.h>

@implementation AppDelegate
{
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Picker did end: %@", identityPicker);
}

- (IBAction)showIdentityPicker:(id)sender;
{
    CUIIdentityPicker *picker = [CUIIdentityPicker new];
    [picker runModalForWindow:self.window modalDelegate:self didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:) contextInfo:NULL];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
}

@end
