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
#import <CredUI/CUIProxyCredential.h>

#import "CUIVBIdentityPickerInternal.h"
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

- (void)dealloc
{
    OM_uint32 minor;

    if (self.usageScenario == kCUIUsageScenarioNetwork && _context != GSS_C_NO_CONTEXT)
        gss_delete_sec_context(&minor, (gss_ctx_id_t *)&_context, GSS_C_NO_BUFFER);

#if !__has_feature(objc_arc)
    [super dealloc];
#endif
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
            [(NSMutableDictionary *)vbCredential.attributes
             setObject:[NSNumber numberWithBool:[self.selectedCredential authenticateForLoginScenario]]
                forKey:(__bridge id)kCUIAttrAuthenticatedForLoginScenario];
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
    if (self.usageScenario == kCUIUsageScenarioNetwork && self.context) {
        NSData *exportedContext = _CUIExportGSSSecContext(&_context);
        if (exportedContext)
            [self.bridge setObject:exportedContext forKey:_CUIIdentityPickerServiceBridgeKeyGSSExportedContext];
    }
}

#pragma mark - Accessors

@synthesize bridge = _bridge;

- (void)setGSSExportedContext:(NSData *)exportedContext
{
    OM_uint32 minor;

    NSAssert(self.usageScenario == kCUIUsageScenarioNetwork, @"GSS context can only be set for kCUIUsageScenarioNetwork");

    if (_context != GSS_C_NO_CONTEXT)
        gss_delete_sec_context(&minor, (gss_ctx_id_t *)&_context, GSS_C_NO_BUFFER);

    _context = _CUIImportGSSSecContext(exportedContext);
    [super setContext:_context];
}

- (void)setPAMSerializedHandle:(NSData *)handleData
{
    NSAssert(self.usageScenario == kCUIUsageScenarioLogin, @"PAM handle can only be set for kCUIUsageScenarioLogin");
    NSAssert(0, @"PAM support not implemented yet for bridge");
}

@end
