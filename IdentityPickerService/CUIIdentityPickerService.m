//
//  CUIIdentityPickerService.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <CredUI/CredUI.h>
#import <CredUI/CUIVBIdentityPicker.h>
#import <CredUI/CUIProxyCredential.h>

#import <CredUICore/CredUICore.h>

#import <CredUI/CUIIdentityPickerInternal.h>

#import "CUIIdentityPickerListenerDelegate.h"
#import "CUIIdentityPickerService.h"
#import "CUIVBIdentityPickerInternal.h"
#import "CUIProxyCredential+AutoAcquire.h"

@interface CUIIdentityPickerService ()
- (BOOL)configureIdentityPicker:(NSArray *)options;
- (void)registerBridgeKeys;
- (void)registerObservers;
- (void)unregisterObservers;

@end

@implementation CUIIdentityPickerService

@synthesize identityPicker = _identityPicker;

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

    return [self.identityPicker configureForUsageScenario:usageScenario flags:flags];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    id value = [change objectForKey:NSKeyValueChangeNewKey];

    if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration]) {
        if ([value isEqual:@YES]) {
            [self.identityPicker startCredentialEnumeration];
        } else {
            [self.identityPicker endCredentialEnumeration:[value integerValue]];
        }
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyConfigOptions]) {
        NSAssert(self.marshal.bridgePhase == NSViewBridgePhaseConfig, @"identity picker can only be configured during config phase");
        [self configureIdentityPicker:value];
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

    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyPersist                  defaultObject:nil owner:NSViewBridgeKeyOwnerPhased];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyGSSExportedContext       defaultObject:nil owner:NSViewBridgeKeyOwnerPhased];

    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyReturnCode               defaultObject:nil owner:NSViewBridgeKeyOwnerService];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeyLastError                defaultObject:nil owner:NSViewBridgeKeyOwnerService];
    [self.bridge registerKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential       defaultObject:nil owner:NSViewBridgeKeyOwnerService];
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
    CUIIdentityPickerInternal *identityPicker;

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

@end
