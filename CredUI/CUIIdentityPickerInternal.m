//
//  CUIIdentityPickerInternalWindowController.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInternal ()

@property(nonatomic, assign) CUIControllerRef controllerRef;
@property(nonatomic, assign) CUICredUIContext *credUIContext;
@property(nonatomic, assign) CUIFlags flags;

@property(nonatomic, retain) NSArrayController *credsController;
@property(nonatomic, assign) BOOL runningModal;
@property(nonatomic, assign) BOOL autoLogin;

@property(nonatomic, retain, readonly) NSString *targetHostName;

- (CUIControllerRef)_newCUIController:(CUIUsageScenario)usageScenario;
- (void)_populateCredentials;
- (void)_updateSubmitButtonForSelectedCred;

@end

@implementation CUIIdentityPickerInternal

- (void)dealloc
{
    if (_controllerRef)
        CFRelease(_controllerRef);
}

- (CUIControllerRef)_newCUIController:(CUIUsageScenario)usageScenario
{
    CUIUsageFlags usageFlags = 0;
    
    if (self.flags & CUIFlagsGenericCredentials)
        usageFlags |= kCUIUsageFlagsGeneric;
    if (self.flags & CUIFlagsExcludeCertificates)
        usageFlags |= kCUIUsageFlagsExcludeCertificates;
    if (self.flags & (CUIFlagsRequireCertificate | CUIFlagsRequireSmartcard))
        usageFlags |= kCUIUsageFlagsRequireCertificates;
    if (self.flags & CUIFlagsPasswordOnlyOK)
        usageFlags |= kCUIUsageFlagsPasswordOnlyOK;
    if (self.flags & CUIFlagsKeepUsername)
        usageFlags |= kCUIUsageFlagsKeepUsername;
    if (self.flags & CUIFlagsExcludePersistedCredentials)
        usageFlags |= kCUIUsageFlagsExcludePersistedCreds;
    
    return CUIControllerCreate(kCFAllocatorDefault, usageScenario, usageFlags);
}

- (instancetype)initWithFlags:(CUIFlags)flags
                usageScenario:(CUIUsageScenario)usageScenario
                   attributes:(NSDictionary *)attributes
{
    self = [super initWithWindow:[self _newPanel]];
    if (self == nil)
        return nil;
    
    if (usageScenario == kCUIUsageScenarioLogin) {
        /* Make sure login credentails can never be persisted */
        flags &= ~(CUIFlagsPersist);
        flags |= CUIFlagsDoNotPersist;
    }

    self.flags = flags;

    if (self.flags & CUIFlagsPersist)
        self.persist = YES;
    else if (self.flags & CUIFlagsDoNotPersist)
        self.persist = NO;
    else
        self.flags |= CUIFlagsShowSaveCheckBox;
    
    self.controllerRef = [self _newCUIController:usageScenario];
    if (self.controllerRef == nil) {
        NSLog(@"Failed to initialize CUIController");
        return nil;
    }
    
    if (attributes)
        self.attributes = attributes;
    
    [self _loadViews];
    
    return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqualTo:@"selectionIndexes"]) {
        NSUInteger index;
        NSArray *creds = [object content];
        NSIndexSet *indexes = [object selectionIndexes];
       
        for (index = 0; index < creds.count; index++) {
            CUICredential *cred = creds[index];
            BOOL autoLogin = NO;
            
            if ([indexes containsIndex:index])
                [cred didBecomeSelected:&autoLogin];
            else
                [cred didBecomeDeselected];
            
            /*
             * This test is to ensure that autoLogin is only invoked if the user explicitly
             * selected a credential (observing a changed, not initial, value), unless there
             * was only one credential.
             */
            if (autoLogin && creds.count > 1 && !self.runningModal)
                autoLogin = NO;
            
            if (autoLogin) {
                NSAssert(self.autoLogin == NO, @"Only one credential can be selected and auto-login");
                self.autoLogin = autoLogin;
                [NSApp stopModal];
            }
            
            [self _updateSubmitButtonForSelectedCred];
        }
    }
}

- (void)_populateCredentials
{
    self.credsController = [[NSArrayController alloc] init];
    self.credsController.selectsInsertedObjects = NO;
    
    [self.collectionView bind:NSContentBinding toObject:self.credsController withKeyPath:@"arrangedObjects" options:nil];
    [self.collectionView bind:NSSelectionIndexesBinding toObject:self.credsController withKeyPath:@"selectionIndexes" options:nil];
    
    CUIControllerEnumerateCredentials(self.controllerRef, ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef error) {
        if (cred) {
            [self.credsController addObject:(__bridge CUICredential *)cred];
            if (isDefault)
                self.credsController.selectionIndex = [self.credsController.arrangedObjects count] - 1;
        } else if (error) {
            NSLog(@"CUIControllerEnumerateCredentials: %@", error);
            self.lastError = CFBridgingRelease(CFRetain(error));
        }
    });
    
    if (self.credsController.selectionIndex == NSNotFound)
        self.credsController.selectionIndex = 0;

    [self.collectionView addObserver:self
                          forKeyPath:@"selectionIndexes"
                             options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                             context:NULL];
}

#pragma mark - Run Loop

- (NSModalResponse)_runModal:(NSWindow *)window
{
    __block NSModalResponse modalResponse = NSModalResponseStop;
    
    if (self.title)
        self.window.title = self.title;
    if (self.message)
        self.messageTextField.stringValue = self.message;
    
    [self _populateCredentials];
   
    /*
     * Generic credentials can be automatically submitted if there is an available
     * credential. Or at least, that's what Windows does.
     */
    if ((self.flags & CUIFlagsGenericCredentials) &&
        (self.flags & CUIFlagsAlwaysShowUI) == 0 &&
        [self.selectedCredential canSubmit])
        goto autoSubmit;

    self.autoLogin = NO;
    self.runningModal = YES;
    
    if (window) {
        [window beginSheet:self.window completionHandler:^(NSModalResponse sheetReturnCode) {
            modalResponse = sheetReturnCode;
        }];
    } else {
        modalResponse = [NSApp runModalForWindow:self.window];
    }
    
    self.runningModal = NO;
    
    if (self.autoLogin) {
    autoSubmit:
        [self willSubmitCredential:self.submitButton];
    }
    
    [self.collectionView removeObserver:self forKeyPath:@"selectionIndexes"];
    [self.credsController unbind:NSContentBinding];
    [self.credsController unbind:NSSelectionIndexesBinding];
    
    [self didSubmitCredential];
    
    return modalResponse;
}

#pragma mark - Credential submission

- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp stopModalWithCode:self.submitButton.state ? NSModalResponseStop : NSModalResponseAbort];
}

- (void)_updateSubmitButtonForSelectedCred
{
    self.submitButton.enabled = [self.selectedCredential canSubmit];
}

- (void)willSubmitCredential:(id)sender
{
    self.submitButton.state = NSOnState; // in case cred was submitted without clicking
    [self.selectedCredential willSubmit];
    [self.window performClose:sender];
}

- (void)didSubmitCredential
{
    [self.selectedCredential didSubmit];
    
    if (self.persist &&
        (self.flags & CUIFlagsExpectConfirmation) == 0)
        [self.selectedCredential savePersisted:NULL];
}

- (void)credentialFieldDidChange:(CUICredential *)credential
{
    if ([credential isEqual:self.selectedCredential])
        [self _updateSubmitButtonForSelectedCred];
}

- (void)didClickPersist:(id)sender
{
    _persist = ((NSButton *)sender).state;
}

- (void)setPersist:(BOOL)persist
{
    _persist = persist;
    [self.persistCheckBox setState:persist];
}

#pragma mark - UI context accessors

- (void)setCredUIContext:(CUICredUIContext *)uic properties:(CUICredUIContextProperties)props
{
    if (uic)
        CUIControllerSetCredUIContext(self.controllerRef, props, uic);
}

- (void)setCredUIContext:(CUICredUIContext *)uic
{
    [self setCredUIContext:uic properties:kCUICredUIContextPropertyAll];
}

- (CUICredUIContext *)credUIContext
{
    return (CUICredUIContext *)CUIControllerGetCredUIContext(self.controllerRef);
}

- (NSString *)title
{
    return (__bridge NSString *)self.credUIContext->titleText;
}

- (void)setTitle:(NSString *)aTitle
{
    CUICredUIContext uic = { .version = 0, .titleText = (__bridge CFStringRef)aTitle };
    [self setCredUIContext:&uic properties:kCUICredUIContextPropertyTitleText];
}

- (NSString *)message
{
    return (__bridge NSString *)self.credUIContext->messageText;
}

- (void)setMessage:(NSString *)aMessage
{
    CUICredUIContext uic = { .version = 0, .titleText = (__bridge CFStringRef)aMessage };
    [self setCredUIContext:&uic properties:kCUICredUIContextPropertyMessageText];
}

#pragma mark - Other accessors

- (CUIUsageScenario)usageScenario
{
    return CUIControllerGetUsageScenario(self.controllerRef);
}

- (CUIUsageFlags)usageFlags
{
    return CUIControllerGetUsageFlags(self.controllerRef);
}

- (CUICredential *)selectedCredential
{
    return [[self.credsController selectedObjects] firstObject];
}

- (NSDictionary *)attributes
{
    return (__bridge NSDictionary *)CUIControllerGetAttributes(self.controllerRef);
}

- (void)setAttributes:(NSDictionary *)someAttributes
{
    CUIControllerSetAttributes(self.controllerRef, (__bridge CFDictionaryRef)someAttributes);
}

- (NSError *)authError
{
    return (__bridge NSError *)CUIControllerGetAuthError(self.controllerRef);
}

- (void)setAuthError:(NSError *)someError
{
    CUIControllerSetAuthError(self.controllerRef, (__bridge CFErrorRef)someError);
}

- (const void *)context
{
    return CUIControllerGetContext(self.controllerRef);
}

- (void)setContext:(const void *)aContext
{
    CUIControllerSetContext(self.controllerRef, aContext);
}

- (id)targetName
{
    return (__bridge id)CUIControllerGetTargetName(self.controllerRef);
}

- (void)setTargetName:(id)aTarget
{
    CUIControllerSetTargetName(self.controllerRef, (__bridge CFTypeRef)aTarget);
}

- (NSString *)targetDisplayName
{
    CFTypeRef targetName = CUIControllerGetTargetName(self.controllerRef);
    
    if (targetName)
        return CFBridgingRelease(CUICopyTargetDisplayName(targetName));
    
    return NULL;
}

- (NSString *)targetHostName
{
    CFTypeRef targetName = CUIControllerGetTargetName(self.controllerRef);
    
    if (targetName)
        return CFBridgingRelease(CUICopyTargetHostName(targetName));

    return NULL;
}

@end
