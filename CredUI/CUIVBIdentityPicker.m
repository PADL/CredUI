//
//  CUIVBIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

/* Remote bridge keys */
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyConfigOptions         = @"usageScenarioAndFlags";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyTitle                 = @"title";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyMessage               = @"message";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyAttributes            = @"attributes";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyAuthError             = @"authError";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyTargetName            = @"targetName";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration = @"startCredentialEnumeration";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyInvocation            = @"invocation";

/* Phased bridge keys */
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyPersist               = @"persist";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyExportedContext       = @"exportedContext";

/* Service bridge keys */
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyReturnCode            = @"returnCode";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyLastError             = @"lastError";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeySelectedCredential    = @"selectedCredential";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyInvocationReply       = @"invocationReply";

static NSString * const _CUIIdentityPickerServiceName                               = @"com.padl.CredUI.xpc.IdentityPickerService";

@interface CUIVBIdentityPicker()
@end

@implementation CUIVBIdentityPicker

#pragma mark - Initialization

- (NSPanel *)_newContainingPanel
{
    /* superclass panel only for dimensions */
    NSWindow *templatePanel = [_reserved[0] window];

    NSAssert(templatePanel != nil, @"superclass panel uninitialized");

    NSPanel *panel = [[NSPanel alloc] initWithContentRect:[templatePanel.contentView frame]
                                                styleMask:templatePanel.styleMask
                                                  backing:templatePanel.backingType
                                                    defer:YES];

    return panel;
}

- (NSRemoteView *)_newRemoteView
{
    NSRemoteView *remoteView = [[NSRemoteView alloc] init];

    if (remoteView == nil)
        return nil;

    remoteView.serviceName = _CUIIdentityPickerServiceName;
    remoteView.serviceSubclassName = @"CUIIdentityPickerService";
    remoteView.trustsServiceKeyEvents = YES;    

    NSAssert(remoteView.bridge != nil, @"view bridge uninitialized");

    remoteView.bridge.kvoBuddy = self;
    remoteView.delegate = self;

    return remoteView;
}

- (void)registerObservers
{
    [self.remoteView.bridge.allKeys enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSViewBridgeKeyOwner owner = [self.remoteView.bridge ownerForKey:obj];

        if (owner == NSViewBridgeKeyOwnerService || owner == NSViewBridgeKeyOwnerPhased)
            [self.remoteView.bridge addObserver:self forKeyPath:obj options:NSKeyValueObservingOptionNew context:NULL];
    }];
}

- (void)unregisterObservers
{
    [self.remoteView.bridge.allKeys enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSViewBridgeKeyOwner owner = [self.remoteView.bridge ownerForKey:obj];

        if (owner == NSViewBridgeKeyOwnerService || owner == NSViewBridgeKeyOwnerPhased)
            [self.remoteView.bridge removeObserver:self forKeyPath:obj];
    }];
}

- (BOOL)configureRemoteViewForUsageScenario:(CUIUsageScenario)usageScenario
                                      flags:(CUIFlags)flags
{
    NSRemoteView *remoteView;

    remoteView = [self _newRemoteView];
    if (remoteView == nil)
        return NO;

    self.remoteView = remoteView;
#if !__has_feature(objc_arc) 
    [remoteView release];
#endif

    [self registerObservers];

    NSArray *options = [NSArray arrayWithObjects:[NSNumber numberWithUnsignedInteger:usageScenario], 
                                                 [NSNumber numberWithUnsignedInteger:flags],
                                                 nil];
    [remoteView.bridge setObject:options forKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions];

    self.containingPanel.contentView = self.remoteView;
    self.invocationReplyDict = [NSMutableDictionary dictionary];

    return YES;
}

- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attributes
                                flags:(CUIFlags)flags
{
    if ((self = [super initWithUsageScenario:usageScenario attributes:attributes flags:flags]) == nil)
        return nil;

    NSPanel *panel = [self _newContainingPanel];
    self.containingPanel = panel;
#if !__has_feature(objc_arc) 
    [panel release];
#endif

    [self configureRemoteViewForUsageScenario:usageScenario flags:flags];

    return self;
}

- (void)credentialInvocation:(CUIProxyCredential *)credential
                    selector:(NSString *)selector
                   withReply:(void (^)(NSError *))replyBlock
{
    CUICredentialRemoteInvocation *invocation = [[CUICredentialRemoteInvocation alloc] init];

    invocation.credentialID = credential.UUID;
    invocation.selector = selector;

    [self.remoteView.bridge setObject:invocation forKey:_CUIIdentityPickerServiceBridgeKeyInvocation];
    [self.invocationReplyDict setObject:[replyBlock copy] forKey:invocation.invocationID];

#if !__has_feature(objc_arc) 
    [invocation release];
#endif
}

- (void)dealloc
{
    [self unregisterObservers];

#if !__has_feature(objc_arc)
    [_remoteView release];
    [_containingPanel release];
    [_contextBox release];
    [_invocationReplyDict release];
    [super dealloc];
#endif
}

#pragma mark - KVO

- (void)endWithReturnCode:(NSModalResponse)returnCode
{
    if (self.containingPanel.isSheet) {
        [self.containingPanel.sheetParent endSheet:self.containingPanel returnCode:returnCode];
    } else {
        [NSApp stopModalWithCode:returnCode];
    }
    [self.containingPanel orderOut:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    id value = [change objectForKey:NSKeyValueChangeNewKey];

    if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyReturnCode] &&
        ![value isEqual:[NSNumber numberWithInteger:NSModalResponseStop]]) { // this is just used as a placeholder/initial value
        CUIProxyCredential *proxyCredential = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential];
        proxyCredential.identityPicker = self;
        id exportedContext = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyExportedContext];
        if (![exportedContext isKindOfClass:[NSNull class]])
            [self.contextBox importContext:exportedContext];
        [self endWithReturnCode:[value integerValue]];
        [self.remoteView.bridge setObject:@NO forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyInvocationReply]) {
        void (^replyBlock)(NSError *) = [self.invocationReplyDict objectForKey:[value invocationID]];
        NSAssert(replyBlock != nil, ([NSString stringWithFormat:@"no valid reply block for invocation %@", [[value invocationID] UUIDString]]));
        if (replyBlock) {
            replyBlock([value error]);
            [self.invocationReplyDict removeObjectForKey:[value invocationID]];
        }
    }
}

#pragma mark - Run Loop

- (void)prepareForCredentialEnumeration
{
    [self.remoteView.bridge setObject:@YES forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];

    NSData *exportedContext = [self.contextBox exportContext];
    [self.remoteView.bridge setObject:exportedContext forKey:_CUIIdentityPickerServiceBridgeKeyExportedContext];
    [self.remoteView advanceToRunPhaseIfNeeded];
}

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow
               completionHandler:(void (^)(NSModalResponse returnCode))handler
{
    [self prepareForCredentialEnumeration];
    [sheetWindow beginSheet:self.containingPanel
     completionHandler:^(NSModalResponse returnCode) {
        handler(returnCode);
     }];
}

- (NSInteger)runModal
{
    [self prepareForCredentialEnumeration];
    return [NSApp runModalForWindow:self.containingPanel];
}

#pragma mark - Accessors

- (CUIUsageScenario)usageScenario
{
    NSArray *options = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions];
    NSAssert(options.count == 2, @"invalid options array");

    return [[options objectAtIndex:0] unsignedIntegerValue];
}

- (CUIFlags)flags
{
    NSArray *options = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions];
    NSAssert(options.count == 2, @"invalid options array");

    return [[options objectAtIndex:1] unsignedIntegerValue];
}

@synthesize remoteView = _remoteView;
@synthesize containingPanel = _containingPanel;
@synthesize contextBox = _contextBox;
@synthesize invocationReplyDict = _invocationReplyDict;

- (NSString *)title
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyTitle];
}

- (void)setTitle:(NSString *)title
{
    [self.remoteView.bridge setObject:title forKey:_CUIIdentityPickerServiceBridgeKeyTitle];
}

- (NSDictionary *)attributes
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyAttributes];
}

- (void)setAttributes:(NSDictionary *)attributes
{
    [self.remoteView.bridge setObject:attributes forKey:_CUIIdentityPickerServiceBridgeKeyAttributes];
}

- (NSError *)authError
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyAuthError];
}

- (void)setAuthError:(NSError *)authError
{
    [self.remoteView.bridge setObject:authError forKey:_CUIIdentityPickerServiceBridgeKeyAuthError];
}

- (BOOL)persist
{
    return [[self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyPersist] boolValue];
}

- (void)setPersist:(BOOL)persist
{
    [self.remoteView.bridge setObject:[NSNumber numberWithBool:persist]
                               forKey: _CUIIdentityPickerServiceBridgeKeyPersist];
}

- (id)targetName
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyTargetName];
}

- (void)setTargetName:(id)targetName
{
    [self.remoteView.bridge setObject:targetName forKey:_CUIIdentityPickerServiceBridgeKeyTargetName];
}

- (CUICredential *)selectedCredential
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential];
}

- (NSError *)lastError
{
    return [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyLastError];
}

@end

