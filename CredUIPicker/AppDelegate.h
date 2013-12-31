//
//  AppDelegate.h
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class CUIIdentityPicker;

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
- (void)doGSSAPITests:(CUIIdentityPicker *)picker;
- (IBAction)showIdentityPicker:(id)sender;
@end
