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
{
    __weak NSWindow *_window;
    CUIIdentityPicker *_picker;
}

@property (nonatomic, weak) IBOutlet NSWindow *window;
@property (nonatomic, strong) CUIIdentityPicker *picker;

- (IBAction)showIdentityPickerGSSIC:(id)sender;
- (IBAction)showIdentityPickerGSSItem:(id)sender;
- (IBAction)showIdentityPickerGeneric:(id)sender;
- (IBAction)gssAuthWithIdentityPickerFallback:(id)sender;
- (IBAction)showIdentityPickerCert:(id)sender;
- (IBAction)showIdentityPickerLocal:(id)sender;

@end

void testEncodeDecode(id object);
