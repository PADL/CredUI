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
    NSWindow *templatePanel = [_reserved window];

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

    NSXPCInterface *marshalInterface = [[remoteView serviceMarshalConnection] exportedInterface];
    [marshalInterface setClasses:[CUICredential builtinClasses] forSelector:@selector(setRemoteObject:forKey:withReply:) argumentIndex:0 ofReply:NO];

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
    void (^replyBlockCopy)(NSError *) = [replyBlock copy];
    
    invocation.credentialID = credential.UUID;
    invocation.selector = selector;

    [self.remoteView.bridge setObject:invocation forKey:_CUIIdentityPickerServiceBridgeKeyInvocation];
    [self.invocationReplyDict setObject:replyBlockCopy forKey:invocation.invocationID];

#if !__has_feature(objc_arc) 
    [invocation release];
    [replyBlockCopy release];
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
            [self.invocationReplyDict removeObjectForKey:[value invocationID]];
            replyBlock([value error]);
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

@synthesize remoteView = _remoteView;
@synthesize containingPanel = _containingPanel;
@synthesize contextBox = _contextBox;
@synthesize invocationReplyDict = _invocationReplyDict;

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

@dynamic title;
@dynamic message;
@dynamic attributes;
@dynamic authError;
@dynamic flags;
@dynamic targetName;
@dynamic selectedCredential;
@dynamic lastError;

- (BOOL)persist
{
    return [[self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyPersist] boolValue];
}

- (void)setPersist:(BOOL)persist
{
    [self.remoteView.bridge setObject:[NSNumber numberWithBool:persist]
                               forKey: _CUIIdentityPickerServiceBridgeKeyPersist];
}

static void _CUIVBIdentityPickerForwardPropertySetter(CUIVBIdentityPicker *self, SEL _cmd, id arg)
{
    NSString *propertyName = _CUIIdentityPickerPropertyForSelector(_cmd);
    NSViewBridgeKeyOwner owner = [self.remoteView.bridge ownerForKey:propertyName];

    NSAssert([self.remoteView.bridge hasKey:propertyName], ([NSString stringWithFormat:@"remoteView missing property %@", propertyName]));
    NSAssert(owner == NSViewBridgeKeyOwnerRemote || owner == NSViewBridgeKeyOwnerPhased, @"can only set properties that are remote or phased");

    [self.remoteView.bridge setObject:arg forKey:propertyName];
}

static id _CUIVBIdentityPickerForwardPropertyGetter(CUIVBIdentityPicker *self, SEL _cmd)
{
    NSString *propertyName = _CUIIdentityPickerPropertyForSelector(_cmd);

    NSAssert([self.remoteView.bridge hasKey:propertyName], ([NSString stringWithFormat:@"remoteView missing property %@", propertyName]));
    
    return [self.remoteView.bridge objectForKey:propertyName];
}

+ (BOOL)resolveInstanceMethod:(SEL)aSEL
{
    BOOL bResolves;
    
    /* forward all properties to bridge */
    if (_CUIIdentityPickerIsValidProperty(aSEL)) {
        if ([NSStringFromSelector(aSEL) hasPrefix:@"set"])
            class_addMethod(self, aSEL, (IMP)_CUIVBIdentityPickerForwardPropertySetter, "v@:@");
        else if (class_getProperty([self class], sel_getName(aSEL)))
            class_addMethod(self, aSEL, (IMP)_CUIVBIdentityPickerForwardPropertyGetter, "@@:");
        bResolves = YES;
    } else {
        bResolves = [super resolveInstanceMethod:aSEL];
    }

    return bResolves;
}

@end
