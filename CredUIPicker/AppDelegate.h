//
//  AppDelegate.h
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#define CREDUIPICKER_TEST_PERSONA 1

#if CREDUIPICKER_TEST_PERSONA
NSString *
PersonaGetAssertion(
                    NSString *audience,
                    NSWindow *parentWindow,
                    NSError * __autoreleasing *error);
#endif

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
#if CREDUIPICKER_TEST_PERSONA
- (IBAction)showPersonaDialog:(id)sender;
#endif
- (IBAction)showIdentityPicker:(id)sender;
@end
