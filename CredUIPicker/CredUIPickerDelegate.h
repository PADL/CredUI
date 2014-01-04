//
//  CredUIPickerDelegate.h
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CUIIdentityPicker;
@class GSSCredential;

@interface CredUIPickerDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
- (GSSCredential *)acquireGSSCred:(CUIIdentityPicker *)identityPicker;

- (IBAction)showIdentityPickerGSSIC:(id)sender;
- (IBAction)showIdentityPickerGSSItem:(id)sender;
- (IBAction)showIdentityPickerGeneric:(id)sender;
- (IBAction)gssAuthWithIdentityPickerFallback:(id)sender;

@end
