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

#pragma mark - Properties

@synthesize containingPanel = _containingPanel;
@synthesize remoteView = _remoteView;
@synthesize contextBox = _contextBox;
@synthesize invocationReplyDict = _invocationReplyDict;

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

- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attributes
                                flags:(CUIFlags)flags
{
    if ((self = [super initWithUsageScenario:usageScenario attributes:attributes flags:flags]) == nil)
        return nil;
    
    _remoteView = [[NSRemoteView alloc] init];
    if (_remoteView == nil)
        return nil;

    _usageScenario = usageScenario;

    NSAssert(self.remoteView != nil, @"remoteView uninitialized");

    self.remoteView.serviceName = _CUIIdentityPickerServiceName;
    self.remoteView.serviceSubclassName = @"CUIIdentityPickerService";
    self.remoteView.trustsServiceKeyEvents = YES;    

    NSAssert(self.remoteView.bridge != nil, @"view bridge uninitialized");
  
    NSPanel *panel = [self _newContainingPanel];
    panel.contentView = self.remoteView;
    self.containingPanel = panel;
#if !__has_feature(objc_arc) 
    [panel release];
#endif

    self.remoteView.bridge.kvoBuddy = self;
    self.remoteView.delegate = self;

    NSArray *options = [NSArray arrayWithObjects:[NSNumber numberWithUnsignedInteger:usageScenario], 
                                                 [NSNumber numberWithUnsignedInteger:flags],
                                                 nil];

    [self.remoteView.bridge setObject:options forKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions];

    [self registerObservers];

    self.invocationReplyDict = [NSMutableDictionary dictionary];

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
    self.containingPanel.contentView = nil;
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    id value = [change objectForKey:NSKeyValueChangeNewKey];

    if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyReturnCode]) {
        CUIProxyCredential *proxyCredential = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeySelectedCredential];
        proxyCredential.identityPicker = self;
        [self.contextBox importContext:[self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyExportedContext]];
        [self endWithReturnCode:[value integerValue]];
        [self.remoteView.bridge setObject:@NO forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];
    } else if ([keyPath isEqual:_CUIIdentityPickerServiceBridgeKeyInvocationReply]) {
        void (^replyBlock)(NSError *) = [self.invocationReplyDict objectForKey:[value invocationID]];
        replyBlock([value error]);
    }
}

#pragma mark - Run Loop

- (void)prepareForCredentialEnumeration
{
    [self.remoteView.bridge setObject:@YES forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];

    NSData *exportedContext = [self.contextBox exportContext];
    if (exportedContext)
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

- (CUIFlags)flags
{
    NSArray *options = [self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyConfigOptions];

    NSAssert(options.count == 2, @"invalid options array");

    return [[options objectAtIndex:1] unsignedIntegerValue];
}
        
@end

