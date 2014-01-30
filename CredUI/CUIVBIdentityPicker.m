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

/* Phased bridge keys */
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyPersist               = @"persist";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyGSSExportedContext    = @"GSSExportedContext";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyPAMSerializedHandle   = @"PAMSerializedHandle";

/* Service bridge keys */
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyReturnCode            = @"returnCode";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeyLastError             = @"lastError";
CUI_EXPORT NSString * const _CUIIdentityPickerServiceBridgeKeySelectedCredential    = @"selectedCredential";

static NSString * const _CUIIdentityPickerServiceName                               = @"com.padl.CredUI.xpc.IdentityPickerService";

@interface CUIVBIdentityPicker()
@end

@implementation CUIVBIdentityPicker

#pragma mark - Properties

@synthesize containingPanel = _containingPanel;
@synthesize remoteView = _remoteView;

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

    return self;
}

- (void)dealloc
{
    [self unregisterObservers];

    switch (_usageScenario) {
        case kCUIUsageScenarioNetwork: {
            OM_uint32 minor;
            gss_delete_sec_context(&minor, (gss_ctx_id_t *)&_context, GSS_C_NO_BUFFER);
            break;
        }
        default:
            break;
    }

#if !__has_feature(objc_arc) 
    [_remoteView release];
    [_containingPanel release];
    [super dealloc];
#endif
}

- (void)setGSSExportedContext:(NSData *)exportedContext
{
    OM_uint32 minor;

    NSAssert(_usageScenario == kCUIUsageScenarioNetwork, @"GSS context can only be set for kCUIUsageScenarioNetwork");

    if (_context != GSS_C_NO_CONTEXT)
        gss_delete_sec_context(&minor, (gss_ctx_id_t *)&_context, GSS_C_NO_BUFFER);

    _context = _CUIImportGSSSecContext(exportedContext);
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
        switch (_usageScenario) {
        case kCUIUsageScenarioNetwork:
            [self setGSSExportedContext:[self.remoteView.bridge objectForKey:_CUIIdentityPickerServiceBridgeKeyGSSExportedContext]];
            break;
        default:
            break;
        }
        [self endWithReturnCode:[value integerValue]];
        [self.remoteView.bridge setObject:@NO forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];
    }
}

#pragma mark - Run Loop

- (void)prepareForCredentialEnumeration
{
    [self.remoteView.bridge setObject:@YES forKey:_CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration];
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

- (const void *)context
{
    return _context;
}

- (void)setContext:(const void *)context
{
    switch (_usageScenario) {
        case kCUIUsageScenarioNetwork: {
            NSData *contextData = _CUIExportGSSSecContext((void **)&context);
            [self.remoteView.bridge setObject:contextData forKey:_CUIIdentityPickerServiceBridgeKeyGSSExportedContext];
#if 0
            // because we killed the context, import it again
            _context = _CUIImportGSSSecContext(contextData);
#endif
            break;
        }
        default:
            break;
    }
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

#pragma mark - Helpers

CUI_EXPORT NSData *
_CUIExportGSSSecContext(void **context)
{
    OM_uint32 major, minor;
    gss_buffer_desc exportedContext = GSS_C_EMPTY_BUFFER;
    NSData *data;

    major = gss_export_sec_context(&minor, (gss_ctx_id_t *)context, &exportedContext);
    if (GSS_ERROR(major))
        return nil;

    data = [NSData dataWithBytes:exportedContext.value length:exportedContext.length];
    gss_release_buffer(&minor, &exportedContext);

    return data;
}

CUI_EXPORT void *
_CUIImportGSSSecContext(NSData *data)
{
    OM_uint32 minor;
    gss_ctx_id_t context = GSS_C_NO_CONTEXT;
    gss_buffer_desc exportedContext;

    if (!data.length)
        return nil;

    exportedContext.length = data.length;
    exportedContext.value = (void *)data.bytes;

    gss_import_sec_context(&minor, &exportedContext, &context);

    return context;
}
