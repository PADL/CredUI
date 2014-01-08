//
//  CUIIdentityPickerInternalWindowController.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInternal ()
- (CUIControllerRef)_newCUIController:(CUIFlags)flags;
@end

@implementation CUIIdentityPickerInternal

- (void)dealloc
{
    if (_controller)
        CFRelease(_controller);
}

- (CUIControllerRef)_newCUIController:(CUIFlags)flags
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
    
    return CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, usageFlags);
}

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes
{
    NSPanel *panel = [self _newPanel];
    
    self = [super initWithWindow:panel];
    if (self == nil)
        return nil;
    
    _flags = flags;
    self.controller = [self _newCUIController:flags];
    if (attributes)
        self.attributes = attributes;
    
    self.messageTextField = [self _newMessageTextField];
    [self.window.contentView addSubview:self.messageTextField];
    
    self.collectionView = [self _newCollectionViewWithWindow:self.window];
    [self.window.contentView addSubview:self.collectionView];
  
    if (self.flags & CUIFlagsPersist)
        self.persist = YES;
    else if (self.flags & CUIFlagsDoNotPersist)
        self.persist = NO;
    else
        _flags |= CUIFlagsShowSaveCheckBox;

    if (self.flags & CUIFlagsShowSaveCheckBox) {
        self.persistCheckBox = [self _newPersistCheckBox];
        [self.window.contentView addSubview:self.persistCheckBox];
    }
    self.submitButton = [self _newSubmitButton];
    [self.window.contentView addSubview:self.submitButton];
    
    CUICredUIContext uic = { .version = 0, .parentWindow = (__bridge CFTypeRef)self.window };
    [self setCredUIContext:&uic properties:kCUICredUIContextPropertyParentWindow];

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
            
            if (autoLogin) {
                NSAssert(self.autoLogin == NO, @"Only one credential can be selected and auto-login");
                self.autoLogin = autoLogin;
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
    
    CUIControllerEnumerateCredentials(_controller, ^(CUICredentialRef cred, CFErrorRef error) {
        if (cred) {
            [self.credsController addObject:(__bridge CUICredential *)cred];
        } else if (error) {
            NSLog(@"CUIControllerEnumerateCredentials: %@", error);
            self.lastError = CFBridgingRelease(CFRetain(error));
        }
    });
    
    self.credsController.selectionIndex = 0;
    
    [self.collectionView addObserver:self
                          forKeyPath:@"selectionIndexes"
                             options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                             context:nil];
}

#pragma mark - Run Loop

- (NSModalResponse)_runModal:(NSWindow *)window
{
    __block NSModalResponse modalResponse = NSModalResponseStop;
    NSModalSession modalSession = NULL;
    
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

    if (window) {
        [window beginSheet:self.window completionHandler:^(NSModalResponse sheetReturnCode) {
            modalResponse = sheetReturnCode;
        }];
    } else {
        modalSession = [NSApp beginModalSessionForWindow:self.window];
        do {
            modalResponse = [NSApp runModalSession:modalSession];
            [[NSRunLoop currentRunLoop] limitDateForMode:NSDefaultRunLoopMode];
            if (self.autoLogin) {
autoSubmit:
                [self willSubmitCredential:self.submitButton];
                self.autoLogin = NO;
            }
        } while (modalResponse == NSModalResponseContinue);
        
        if (modalSession)
            [NSApp endModalSession:modalSession];
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
    CUIControllerSetCredUIContext(_controller, props, uic);
}

- (void)setCredUIContext:(CUICredUIContext *)uic
{
    [self setCredUIContext:uic properties:kCUICredUIContextPropertyAll];
}

- (CUICredUIContext *)credUIContext
{
    return (CUICredUIContext *)CUIControllerGetCredUIContext(_controller);
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

- (CUICredential *)selectedCredential
{
    return [[self.credsController selectedObjects] firstObject];
}

- (NSDictionary *)attributes
{
    return (__bridge NSDictionary *)CUIControllerGetAttributes(_controller);
}

- (void)setAttributes:(NSDictionary *)someAttributes
{
    CUIControllerSetAttributes(_controller, (__bridge CFDictionaryRef)someAttributes	);
}

- (GSSContext *)GSSContextHandle
{
    return (__bridge GSSContext *)CUIControllerGetGSSContextHandle(_controller);
}

- (void)setGSSContextHandle:(GSSContext *)aContext
{
    CUIControllerSetGSSContextHandle(_controller, (__bridge CFTypeRef)aContext);
}

- (id)targetName
{
    return (__bridge id)CUIControllerGetTargetName(_controller);
}

- (void)setTargetName:(id)aTarget
{
    CUIControllerSetTargetName(_controller, (__bridge CFTypeRef)aTarget);
}

- (NSString *)targetDisplayName
{
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    
    if (targetName)
        return CFBridgingRelease(CUICopyTargetDisplayName(targetName));
    
    return NULL;
}

- (NSString *)targetHostName
{
    CFTypeRef targetName = CUIControllerGetTargetName(_controller);
    
    if (targetName)
        return CFBridgingRelease(CUICopyTargetHostName(targetName));

    return NULL;
}

@end
