//
//  CredUIPickerDelegate.m
//  CredUIPicker
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <GSSKitUI/GSSPromptForCredentials.h>
#import <CredUI/CUICredential+CBIdentity.h>
#import <CredUICore/CUIAttributes.h>

#import <GSSKit/GSSKit.h>

#import <GSSKitUI/GSSKitUI.h>
#import <dispatch/dispatch.h>

#import "GSSItem.h"

#import "CredUIPickerDelegate.h"
#import "CredUIPickerDelegate+InitAcceptLoop.h"

@implementation CredUIPickerDelegate

@synthesize window = _window;
@synthesize picker = _picker;

#pragma mark - GSS IC picker

/*
 * Test encoding/decoding of credentials
 */
void testEncodeDecode(id object)
{
    NSString *filePath = @"/tmp/testEncodeDecode.dat";
    
    [NSKeyedArchiver archiveRootObject:object toFile:filePath];
    
    id object2 = [NSKeyedUnarchiver unarchiveObjectWithData:[NSData dataWithContentsOfFile:filePath]];
    
    NSLog(@"reconstituted object: %@", object2);
}

- (void)pickerWithFlags:(CUIFlags)flags
{
    [self pickerWithUsageScenario:kCUIUsageScenarioNetwork attributes:nil flags:flags];
}

- (void)pickerWithAttributes:(NSDictionary *)attributes
                       flags:(CUIFlags)flags
{
    [self pickerWithUsageScenario:kCUIUsageScenarioNetwork attributes:attributes flags:flags];
}

- (void)pickerWithUsageScenario:(CUIUsageScenario)usageScenario
                     attributes:(NSDictionary *)attributes
                          flags:(CUIFlags)flags
{
    CUIIdentityPicker *picker;

    picker = [[CUIIdentityPicker alloc] initWithUsageScenario:usageScenario attributes:attributes flags:flags];
    self.picker = picker;
#if !__has_feature(objc_arc)
    [picker release];
#endif
}

- (void)identityPickerDidEndGSSIC:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseOK) {
        //testEncodeDecode(identityPicker.selectedCredential);
        NSLog(@"IC picker did end: %@", identityPicker.selectedCredential.attributes);
        (void) [self doInitAcceptGSSContextWithIdentityPicker:identityPicker];
    } else {
        NSLog(@"IC picker aborted");
    }
}

- (IBAction)showIdentityPickerGSSIC:(id)sender
{
    [self pickerWithFlags:CUIFlagsExcludePersistedCredentials | CUIFlagsExcludeCertificates | CUIFlagsDoNotPersist];
    
//  self.picker.attributes = @{ (__bridge id)kCUIAttrClass : (__bridge id)kCUIAttrClassKerberos };
//  self.picker.attributes = @{ (__bridge id)kCUIAttrClass : @"1.3.6.1.4.1.5322.24.1.17" };
    
//    self.picker.title = @"Identity Picker";
    self.picker.message = @"Choose an identity";

    /* The target name can be a NSString, NSURL or gss_name_t */
    GSSName *targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    GSSName *personaName = [targetName mechanismName:[GSSMechanism personaMechanism]];
    self.picker.targetName = personaName ? personaName : targetName;

    [self.picker beginSheetModalForWindow:self.window
                            modalDelegate:self
                           didEndSelector:@selector(identityPickerDidEndGSSIC:returnCode:contextInfo:)
                              contextInfo:NULL];
}

#pragma mark - GSS Item picker

- (void)identityPickerDidEndGSSItem:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseOK) {
        NSLog(@"Item picker did end: %@", identityPicker.selectedCredential.attributes);
        __block NSError *error = nil;
        
        // OK, now let's try and do some GSS stuff
        GSSItem *item = CFBridgingRelease([identityPicker.selectedCredential copyMatchingGSSItem:NO error:&error]);
        
        NSLog(@"*** GSS Item Test: %@", item);
        
        NSMutableDictionary *attrs = [[identityPicker.selectedCredential attributesWithClass:CUIAttributeClassGSSItem] mutableCopy];

        NSLog(@"acquire attrs %@", attrs);
        GSSCredential *cred = [item acquire:attrs error:&error];
        if (cred) {
            NSLog(@"Acquire item %@: %@", item, cred);
        } else {
            NSLog(@"Acquire item %@ failed: %@", item, error);
        }
        
        NSLog(@"*** GSS Cred Test");
        cred = [[GSSCredential alloc] initWithCUICredential:identityPicker.selectedCredential error:&error];
        if (cred) {
            NSLog(@"Acquire credential based on item %@: %@", item, cred);
        } else {
            NSLog(@"Acquire credential based on item %@ failed: %@", item, error);
        }
#if !__has_feature(objc_arc)
        [cred release];
        [attrs release];
#endif
    }
}

- (IBAction)showIdentityPickerGSSItem:(id)sender
{
    [self pickerWithFlags:CUIFlagsExcludeCertificates];
    
    self.picker.title = @"Item Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"http://www.padl.com"];
//    self.picker.persist = YES;
    
    [self.picker beginSheetModalForWindow:self.window
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
    [self pickerWithFlags:CUIFlagsGenericCredentials | CUIFlagsAlwaysShowUI | CUIFlagsExcludeCertificates];
    
    self.picker.title = @"Generic Identity Picker";
    self.picker.message = @"Choose an identity";
    self.picker.targetName = [NSURL URLWithString:@"https://www.padl.com"];
    
    [self.picker beginSheetModalForWindow:self.window
                            modalDelegate:self
                           didEndSelector:@selector(identityPickerDidEndGeneric:returnCode:contextInfo:)
                              contextInfo:NULL];
}

#pragma mark - GSS fallback picker

- (IBAction)gssAuthWithIdentityPickerFallback:(id)sender
{
    GSSMechanism *mechanism = [GSSMechanism personaMechanism];
    GSSName *targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
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
   
    [self doInitAcceptGSSContext:initiatorCtx];

#if !__has_feature(objc_arc)
    [initiatorCtx release];
    dispatch_release(queue);
#endif
}

- (void)identityPickerDidEndCert:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseOK) {
        testEncodeDecode(identityPicker.selectedCredential);
        NSLog(@"Cert picker did end: %@", identityPicker.selectedCredential.attributes);
        (void) [self doInitAcceptGSSContextWithIdentityPicker:identityPicker];
    } else {
        NSLog(@"Cert picker aborted");
    }
}

- (IBAction)showIdentityPickerCert:(id)sender
{
    [self pickerWithFlags:CUIFlagsExcludePersistedCredentials | CUIFlagsRequireCertificate];
    
    self.picker.title = @"Certificate Picker";
    self.picker.message = @"Choose an identity";
        
    /* The target name can be a NSString, NSURL or gss_name_t */
    self.picker.targetName = [GSSName nameWithHostBasedService:@"host" withHostName:@"rand.mit.de.padl.com"];
    
    [self.picker beginSheetModalForWindow:self.window
                            modalDelegate:self
                           didEndSelector:@selector(identityPickerDidEndCert:returnCode:contextInfo:)
                              contextInfo:NULL];
}

- (void)identityPickerDidEndLocal:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSModalResponseOK) {
        NSLog(@"Local picker did end: %@", identityPicker.selectedCredential.attributes);
        NSLog(@"CB identity: %@", identityPicker.selectedCredential.userIdentity);
        NSLog(@"Authentication status: %d", [identityPicker.selectedCredential authenticateForLoginScenario]);
    } else {
        NSLog(@"Local picker aborted");
    }
}

- (IBAction)showIdentityPickerLocal:(id)sender
{
    [self pickerWithUsageScenario:kCUIUsageScenarioLogin attributes:nil flags:0];
                   
    self.picker.title = @"Local Picker";
    self.picker.message = @"Choose an identity";
    
#if 0
    [self.picker beginSheetModalForWindow:self.window
                            modalDelegate:self
                           didEndSelector:@selector(identityPickerDidEndLocal:returnCode:contextInfo:)
                              contextInfo:NULL];
#else
    NSModalResponse modalResponse = [self.picker runModal];
    [self identityPickerDidEndLocal:self.picker returnCode:modalResponse contextInfo:NULL];
#endif
}

@end
