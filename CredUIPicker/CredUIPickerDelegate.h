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
- (NSUInteger)initAcceptGSSContext:(CUIIdentityPicker *)identityPicker
                     initiatorCred:(GSSCredential *)initiatorCred;

- (GSSCredential *)acquireGSSCred:(CUIIdentityPicker *)identityPicker;

- (IBAction)showIdentityPicker:(id)sender;
@end
