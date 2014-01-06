//
//  CredUIPickerDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/GSSPromptForCredentials.h>
#import <CredUI/CUICredential+GSS.h>
#import <CredUICore/CUIAttributes.h>

#import <GSSKit/GSSKit.h>

#import <GSSKitUI/GSSKitUI.h>
#import <dispatch/dispatch.h>

#import "GSSItem.h"

#import "CredUIPickerDelegate.h"
#import "CredUIPickerDelegate+InitAcceptLoop.h"

@implementation CredUIPickerDelegate

#pragma mark - GSS IC picker

static void testEncodeDecode(CUICredential * cred)
{
    NSString *filePath = @"/tmp/somecred";
    
    [NSKeyedArchiver archiveRootObject:cred toFile:filePath];
    
    id reconstitutedCred = [NSKeyedUnarchiver unarchiveObjectWithData:[NSData dataWithContentsOfFile:filePath]];
    
    NSLog(@"reconstitutedCred: %@", reconstitutedCred);
}

- (void)identityPickerDidEndGSSIC:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseStop) {
        testEncodeDecode(identityPicker.selectedCredential);
        NSLog(@"IC picker did end: %@", identityPicker.selectedCredential.attributes);
        (void) [self doInitAcceptGSSContextWithIdentityPicker:identityPicker];
    } else {
        NSLog(@"IC picker aborted");
    }
}

- (IBAction)showIdentityPickerGSSIC:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsExcludePersistedCredentials];
    
    self.picker.title = @"Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(identityPickerDidEndGSSIC:returnCode:contextInfo:)
                       contextInfo:NULL];
}

#pragma mark - GSS Item picker

- (void)identityPickerDidEndGSSItem:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseStop) {
        NSLog(@"Item picker did end: %@", identityPicker.selectedCredential.attributes);
        __block NSError *error = nil;
        
        // OK, now let's try and do some GSS stuff
        GSSItem *item = CFBridgingRelease([identityPicker.selectedCredential copyMatchingGSSItem:NO error:&error]);
        
        NSLog(@"GSS Item: %@", item);
        
        NSMutableDictionary *attrs = [[identityPicker.selectedCredential attributesWithClass:CUIAttributeClassGSSItem] mutableCopy];

        NSLog(@"acquire attrs %@", attrs);
        id cred = [item acquire:attrs error:&error];
        if (cred) {
            NSLog(@"Acquire item %@: %@", item, cred);
        } else {
            NSLog(@"Acquire item %@ failed: %@", item, error);
        }
    }
}

- (IBAction)showIdentityPickerGSSItem:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:0];
    
    self.picker.title = @"Item Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"http://www.padl.com"];
    self.picker.persist = YES;
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(identityPickerDidEndGSSItem:returnCode:contextInfo:)
                       contextInfo:NULL];

}

#pragma mark - Generic picker

- (void)identityPickerDidEndGeneric:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSLog(@"Generic picker did end: %@", identityPicker.selectedCredential.attributes);
}

- (IBAction)showIdentityPickerGeneric:(id)sender
{
    self.picker = [[CUIIdentityPicker alloc] initWithFlags:CUIFlagsGenericCredentials | CUIFlagsAlwaysShowUI];
    
    self.picker.title = @"Generic Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"https://www.padl.com"];
    
    [self.picker runModalForWindow:self.window
                     modalDelegate:self
                    didEndSelector:@selector(identityPickerDidEndGeneric:returnCode:contextInfo:)
                       contextInfo:NULL];
}

#pragma mark - GSS fallback picker

- (IBAction)gssAuthWithIdentityPickerFallback:(id)sender
{
    GSSName *targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    GSSMechanism *mechanism = [GSSMechanism personaMechanism];
    GSSCredential *cred = [GSSCredential credentialWithName:@"lukeh@padl.com" mechanism:mechanism];
    
    GSSContext *initiatorCtx;
    dispatch_queue_t queue = dispatch_queue_create("com.padl.CredUIPicker.queue", DISPATCH_QUEUE_SERIAL);
    
    initiatorCtx = [[GSSContext alloc] initWithRequestFlags:0 //GSS_C_MUTUAL_FLAG
                                                      queue:queue
                                                isInitiator:YES];
    
    initiatorCtx.mechanism = mechanism;
    initiatorCtx.targetName = targetName;
    initiatorCtx.credential = cred;
    initiatorCtx.promptForCredentials = YES;
    initiatorCtx.window = self.window;
    
    return [self doInitAcceptGSSContext:initiatorCtx];
}

@end
