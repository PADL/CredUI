//
//  CUIVBIdentityPickerInternal.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import "CUIVBIdentityPickerInternal.h"
#import "CUICredential+Private.h"
#import "CUIProxyCredential.h"
#import "CUIProxyCredential+ViewBridge.h"

#import <CredUICore/CredUICore_Private.h>

@interface CUIVBIdentityPickerInternal ()
- (NSSet *)whitelistedAttributeKeys;
@end

@implementation CUIVBIdentityPickerInternal

#pragma mark - Initialization

- (NSSet *)whitelistedAttributeKeys
{
    CFSetRef whitelist = _CUIControllerCopyWhitelistedAttributeKeys(self.controllerRef);

    return CFBridgingRelease(whitelist);
}

- (instancetype)initWithViewBridge:(NSViewBridge *)viewBridge;
{
    if ((self = [super init]) == nil)
        return nil;

    self.bridge = viewBridge;

    return self;
}

#pragma mark - Credential submission overrides

- (void)willCancelCredential:(id)sender
{
    [self.bridge setObject:[NSNumber numberWithInteger:NSModalResponseCancel] forKey:_CUIIdentityPickerServiceBridgeKeyReturnCode];
    [self.bridge setObject:self.lastError forKey:_CUIIdentityPickerServiceBridgeKeyLastError];
}

- (void)bridgeSelectedCredential
{
    CUIProxyCredential *vbCredential;

    vbCredential = [[CUIProxyCredential alloc] initWithCredential:self.selectedCredential
                                                        whitelist:self.whitelistedAttributeKeys
                                                          mutable:YES];

    if ((self.flags & CUIFlagsGenericCredentials) == 0) {
        switch (self.usageScenario) {
        case kCUIUsageScenarioLogin:
            /* try to auth the user */
            [vbCredential authAndSetAuthenticatedForLoginScenario];
            break;
        case kCUIUsageScenarioNetwork:
            /* try to get creds for the user */
            [vbCredential acquireAndSetGSSCredential];
            break;
        default:
            NSAssert(self.usageScenario != kCUIUsageScenarioLogin && self.usageScenario != kCUIUsageScenarioNetwork, @"invalid usage scenario");
            break;
        }
    }

    [self.bridge setObject:vbCredential forKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential];

#if !__has_feature(objc_arc)
    [vbCredential release];
#endif
}

- (void)willSubmitCredential:(id)sender
{
    NSError *error = nil;

    self.submitButton.state = NSOnState; // in case cred was submitted without clicking

    [self.selectedCredential willSubmit];

    if (self.selectedCredential)
        [self bridgeSelectedCredential];
    error = [self.selectedCredential.attributes objectForKey:(__bridge id)kCUIAttrCredentialError];
    if (error == nil)
        error = self.lastError;
    [self.bridge setObject:error forKey:_CUIIdentityPickerServiceBridgeKeyLastError];
    [self.bridge setObject:[NSNumber numberWithBool:self.persist] forKey:_CUIIdentityPickerServiceBridgeKeyPersist];
    [self.bridge setObject:[NSNumber numberWithInteger:NSModalResponseOK] forKey:_CUIIdentityPickerServiceBridgeKeyReturnCode];
}

#pragma mark - Accessors

@synthesize bridge = _bridge;

- (NSData *)GSSExportedContext
{
    return nil;
}

- (void)setGSSExportedContext:(NSData *)exportedContext
{
    NSAssert(exportedContext == nil, @"setting of an exported context is not yet implemented");
}

@end
