//
//  CUIVBIdentityPickerInternal.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/CUICredential+Private.h>
#import <CredUI/CUICredential+CBIdentity.h>
#import <CredUI/CUICredential+Coding.h>
#import <CredUI/CUIProxyCredential.h>
#import <CredUI/CUIContextBoxing.h>

#import "CUIVBIdentityPickerInternal.h"
#import "CUIProxyCredential+ViewBridge.h"

#import <CredUICore/CredUICore_Private.h>

@interface CUIVBIdentityPickerInternal ()
- (NSSet *)whitelistedAttributeKeys;
@end

@implementation CUIVBIdentityPickerInternal

#pragma mark - Accessors

@synthesize bridge = _bridge;

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

- (CUICredential *)credentialWithUUID:(NSUUID *)nsUuid
{
    id cfUuid = [CUICredential CFUUIDFromNSUUID:nsUuid];
    __block CUICredential *credential = nil;

    [self.credsController.arrangedObjects enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        CUICredential *cred = obj;

        if ([[[cred attributes] objectForKey:(__bridge id)kCUIAttrUUID] isEqual:cfUuid]) {
            credential = obj;
            *stop = YES;
        }
    }];

    return credential;
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
        case kCUIUsageScenarioUnlock:
            /* try to auth the user */
            [(NSMutableDictionary *)vbCredential.attributes
             setObject:[NSNumber numberWithBool:[self.selectedCredential authenticateForLoginScenario]]
                forKey:(__bridge id)kCUIAttrAuthenticatedForLoginScenario];
            break;
        case kCUIUsageScenarioNetwork:
            /* try to get creds for the user */
            [vbCredential acquireAndSetGSSCredential];
            break;
        default:
            /* not sure what to do for other usage scenarios, yet */
            break;
        }
    }

    /*
     * Ensure credential has a UUID in case we need to refer to it later.
     */
    if ([vbCredential.attributes objectForKey:(__bridge id)kCUIAttrUUID] == nil) {
        id credUuid = CFBridgingRelease(CFUUIDCreate(kCFAllocatorDefault));
        [(NSMutableDictionary *)vbCredential.attributes setObject:credUuid forKey:(__bridge id)kCUIAttrUUID];
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

    NSData *exportedContext = self.contextBox.exportContext;
    if (exportedContext)
        [self.bridge setObject:exportedContext forKey:_CUIIdentityPickerServiceBridgeKeyExportedContext];
}

@end
