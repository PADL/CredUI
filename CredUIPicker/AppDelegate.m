//
//  AppDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "AppDelegate.h"

#import <CredUI/CUIIdentityPicker.h>

#import <browserid.h>
#import <CFBrowserID.h>

#if CREDUIPICKER_TEST_PERSONA
NSString *
PersonaGetAssertion(
                    NSString *audience,
                    NSWindow *parentWindow,
                    NSError * __autoreleasing *error);
#endif

@interface AppDelegate ()
@property (nonatomic, strong) CUIIdentityPicker *picker;
@end

@implementation AppDelegate

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Picker did end: %@", [identityPicker selectedCredentialAttributes]);
}

#if CREDUIPICKER_TEST_PERSONA
- (IBAction)showPersonaDialog:(id)sender
{
    NSString *assertion;
    NSError *error;
    
    assertion = PersonaGetAssertion(@"host/foo.bar.com", nil, &error);
    NSLog(@"assertion = %@, error = %@", assertion, error);
}
#endif

- (IBAction)showIdentityPicker:(id)sender;
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials];
    
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

#if CREDUIPICKER_TEST_PERSONA

/*
 * Display a modal dialog acquiring an assertion for the given audience.
 */
NSString *
PersonaGetAssertion(
                    NSString *audience,
                    NSWindow *parentWindow,
                    NSError * __autoreleasing *error)
{
    BIDContext context;
    CFStringRef assertion;
    CFErrorRef cfErr;
    uint32_t flags;
    
    context = BIDContextCreate(kCFAllocatorDefault, NULL, BID_CONTEXT_USER_AGENT, &cfErr);
    if (context == NULL) {
        if (error)
            *error = CFBridgingRelease(cfErr);
        else
            CFRelease(cfErr);
        return NULL;
    }
    
    BIDSetContextParam(context, BID_PARAM_PARENT_WINDOW, (__bridge void *)parentWindow);
    
    assertion = BIDAssertionCreateUI(context, (__bridge CFStringRef)audience,
                                     NULL, NULL, 0, NULL, &flags, &cfErr);
    
    if (cfErr) {
        if (error)
            *error = CFBridgingRelease(cfErr);
        else
            CFRelease(cfErr);
    }
    
    CFRelease(context);
    
    return CFBridgingRelease(assertion);
}

#endif /* CREDUIPICKER_TEST_PERSONA */