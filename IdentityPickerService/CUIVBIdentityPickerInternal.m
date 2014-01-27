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
#import "CUIProxyCredential+AutoAcquire.h"

#import <CredUICore/CredUICore_Private.h>

@implementation CUIVBIdentityPickerInternal

#pragma mark - Initialization

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

- (BOOL)bridgeSelectedCredential:(NSError * __autoreleasing *)error
{
    CUIProxyCredential *vbCredential;
    BOOL ret = YES;

    if (error)
        *error = nil;

    vbCredential = [[CUIProxyCredential alloc] initWithCredential:self.selectedCredential
                                                        whitelist:self.whitelistedAttributeKeys
                                                          mutable:YES];

    if ((self.flags & CUIFlagsGenericCredentials) == 0) {
        /* Acquire GSS cred now, because remote client may not have permission */
        ret = [vbCredential autoAcquireGSSCred:error];
    }

    if (ret) {
        [self.bridge setObject:vbCredential forKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential];
    }

#if !__has_feature(objc_arc)
    [vbCredential release];
#endif

    return ret;
}

- (void)willSubmitCredential:(id)sender
{
    NSError *error = nil;

    NSLog(@"willSubmitCredential: %@", self.selectedCredential);
    self.submitButton.state = NSOnState; // in case cred was submitted without clicking

    [self.selectedCredential willSubmit];

    if (self.selectedCredential)
        [self bridgeSelectedCredential:&error];
    if (error == nil)
        error = self.lastError;
    [self.bridge setObject:self.lastError forKey:_CUIIdentityPickerServiceBridgeKeyLastError];
    [self.bridge setObject:[NSNumber numberWithBool:self.persist] forKey:_CUIIdentityPickerServiceBridgeKeyPersist];
    [self.bridge setObject:[NSNumber numberWithInteger:NSModalResponseOK] forKey:_CUIIdentityPickerServiceBridgeKeyReturnCode];

    NSLog(@"Sending credential: %@", [[self.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential] attributes]);
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
