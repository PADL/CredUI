//
//  CUIIdentityPickerService.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <objc/runtime.h>

#import <Cocoa/Cocoa.h>

#import <GSS/GSS.h>

#import <CredUI/CredUI.h>
#import <CredUI/CUIVBIdentityPicker.h>
#import <CredUI/CUIProxyCredential.h>

#import <CredUICore/CredUICore.h>

#import <CredUI/CUIIdentityPickerInternal.h>
#import <CredUI/CUICredentialRemoteInvocation.h>

#import "CUIIdentityPickerListenerDelegate.h"
#import "CUIIdentityPickerService.h"
#import "CUIVBIdentityPickerInternal.h"

@interface CUIIdentityPickerService ()
- (BOOL)configureIdentityPicker:(NSArray *)options;
- (void)registerBridgeKeys;
- (void)registerObservers;
- (void)unregisterObservers;
@end

@implementation CUIIdentityPickerService

- (void)dealloc
{
//    [self unregisterObservers];
#if !__has_feature(objc_arc)
    [_identityPicker release];
    [super dealloc];
#endif
}

- (BOOL)configureIdentityPicker:(NSArray *)options
{
    NSAssert(options.count == 2, @"invalid options array");

    CUIUsageScenario usageScenario = [[options objectAtIndex:0] unsignedIntegerValue];
    CUIFlags flags = [[options objectAtIndex:1] unsignedIntegerValue];

    self.identityPicker.contextBox = self;

    return [self.identityPicker configureForUsageScenario:usageScenario flags:flags];
}

- (void)handleCredentialRemoteInvocation:(CUICredentialRemoteInvocation *)remoteInvocation
{
    CUICredential *credential = [self.identityPicker credentialWithUUID:remoteInvocation.credentialID];
    NSAssert(credential != nil, ([NSString stringWithFormat:@"no credential for UUID %@", remoteInvocation.credentialID.UUIDString]));

    SEL selector = NSSelectorFromString(remoteInvocation.selector);
    NSAssert(protocol_getMethodDescription(@protocol(CUIProxyCredentialRemoteInvocation), selector, YES, YES).name != NULL,
             ([NSString stringWithFormat:@"selector %@ must conform to CUIProxyCredentialRemoteInvocation protocol (invocation %@/%@)",
             remoteInvocation.selector, remoteInvocation.invocationID.UUIDString, remoteInvocation.credentialID.UUIDString]));

    [credential performSelector:selector withObject:^(NSError *error) {
        remoteInvocation.error = error;
    }];

    [self.bridge setObject:remoteInvocation forKey:_CUIIdentityPickerServiceBridgeKeyInvocationReply];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    id value = [change objectForKey:NSKeyValueChangeNewKey];

    if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration]) {
        if ([value isEqual:@YES]) {
            [self.identityPicker startCredentialEnumeration:self.view.window];
        } else {
            [self.identityPicker endCredentialEnumeration:[value integerValue]];
        }
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyConfigOptions]) {
        NSAssert(self.marshal.bridgePhase == NSViewBridgePhaseConfig, @"identity picker can only be configured during config phase");
        [self configureIdentityPicker:value];
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyExportedContext]) {
        BOOL bImportedContext = [self importContext:value];
        NSAssert(bImportedContext, @"failed to import GSS context");
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyInvocation]) {
        [self handleCredentialRemoteInvocation:value];
    } else if ([self.identityPicker isConfigured]) {
        [self.identityPicker setValue:value forKey:keyPath];
    }
}

- (void)registerBridgeKeys
{
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions            defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyTitle                    defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyMessage                  defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyAttributes               defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyAuthError                defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyTargetName               defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyInvocation                defaultObject:nil owner:NSViewBridgeKeyOwnerRemote];

    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyPersist                  defaultObject:nil owner:NSViewBridgeKeyOwnerPhased];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyExportedContext          defaultObject:nil owner:NSViewBridgeKeyOwnerPhased];

    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyReturnCode               defaultObject:nil owner:NSViewBridgeKeyOwnerService];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyLastError                defaultObject:nil owner:NSViewBridgeKeyOwnerService];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential       defaultObject:nil owner:NSViewBridgeKeyOwnerService];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyInvocationReply          defaultObject:nil owner:NSViewBridgeKeyOwnerService];
}

- (void)registerObservers
{
    [self.bridge.allKeys enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSViewBridgeKeyOwner owner = [self.bridge ownerForKey:obj];
        
        if (owner == NSViewBridgeKeyOwnerRemote || owner == NSViewBridgeKeyOwnerPhased)
            [self.bridge addObserver:self forKeyPath:obj options:NSKeyValueObservingOptionNew context:NULL];
    }];
}

- (void)unregisterObservers
{
    [self.bridge.allKeys enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSViewBridgeKeyOwner owner = [self.bridge ownerForKey:obj];
        
        if (owner == NSViewBridgeKeyOwnerRemote || owner == NSViewBridgeKeyOwnerPhased)
            [self.bridge removeObserver:self forKeyPath:obj];
    }];
}

- (NSUInteger)awakeFromRemoteView
{
    NSUInteger response;
    CUIVBIdentityPickerInternal *identityPicker;

    response = [super awakeFromRemoteView];
    
    [self registerBridgeKeys];

    /* attributes can be set later */
    identityPicker = [[CUIVBIdentityPickerInternal alloc] initWithViewBridge:self.bridge];
    if (identityPicker == nil)
        return NO;
    
    self.identityPicker = identityPicker;

    NSAssert(self.identityPicker, @"identityPicker initialized");
    NSAssert(self.identityPicker.window, @"identityPicker panel initialized");

    [self.view addSubview:self.identityPicker.window.contentView];

    [self registerObservers];

#if !__has_feature(objc_arc)
    [identityPicker release];
#endif

    return response;
}

@synthesize identityPicker = _identityPicker;

- (CUIUsageScenario)usageScenario
{
    return self.identityPicker.usageScenario;
}

- (void *)context
{
    return _context;
}

- (void)setContext:(void *)aContext
{
    NSAssert(self.usageScenario == kCUIUsageScenarioNetwork, @"context transferring only supported for kCUIUsageScenarioNetwork");

    if (aContext != _context) {
        OM_uint32 minor;
        gss_delete_sec_context(&minor, (gss_ctx_id_t *)&_context, GSS_C_NO_BUFFER);
        _context = aContext;
    }
}

- (NSData *)exportContext
{
    NSData *data;
    OM_uint32 major, minor;
    gss_buffer_desc exportedContext = GSS_C_EMPTY_BUFFER;
    gss_ctx_id_t context = self.context;

    NSAssert(self.usageScenario == kCUIUsageScenarioNetwork, @"context transferring only supported for kCUIUsageScenarioNetwork");

    major = gss_export_sec_context(&minor, &context, &exportedContext);
    if (GSS_ERROR(major))
        return nil;

    data = [NSData dataWithBytes:exportedContext.value length:exportedContext.length];
    gss_release_buffer(&minor, &exportedContext);

    self.context = GSS_C_NO_CONTEXT;

    return data;
}

- (BOOL)importContext:(NSData *)data
{
    OM_uint32 major, minor;
    gss_buffer_desc exportedContext;
    void *context = GSS_C_NO_CONTEXT;

    if (!data.length)
        return NO;

    exportedContext.length = data.length;
    exportedContext.value = (void *)data.bytes;

    major = gss_import_sec_context(&minor, &exportedContext, (gss_ctx_id_t *)&context);
    if (GSS_ERROR(major))
        return NO;

    self.context = context;

    return YES;
}

@end
