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
    if (self.flags & CUIFlagsExcludePersistedCredentials)
        usageFlags |= kCUIUsageFlagsExcludePersistedCreds;
    if (self.flags & CUIFlagsKeepUsername)
        usageFlags |= kCUIUsageFlagsKeepUsername;
    if (self.flags & CUIPasswordOnlyOK)
        usageFlags |= kCUIUsageFlagsPasswordOnlyOK;
    
    return CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, usageFlags);
}

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes;
{
    NSPanel *panel = [self _newPanel];
    
    self = [super initWithWindow:panel];
    if (self) {
        _flags = flags;
        if (attributes)
            self.attributes = attributes;
        self.controller = [self _newCUIController:flags];
        
        self.messageTextField = [self _newMessageTextField];
        [self.window.contentView addSubview:self.messageTextField];
        
        self.collectionView = [self _newCollectionViewWithWindow:self.window];
        [self.window.contentView addSubview:self.collectionView];
      
        if (self.flags & CUIPersist)
            self.persist = YES;
        else if (self.flags & CUIDoNotPersist)
            self.persist = NO;
        else
            _flags |= CUIShowSaveCheckBox;

        if (self.flags & CUIShowSaveCheckBox)
            [self.window.contentView addSubview:[self _newPersistCheckBox]];
        
        self.submitButton = [self _newSubmitButton];
        [self.window.contentView addSubview:self.submitButton];
    }
    
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
    CUICredUIContext uic = { .version = 0, .parentWindow = (__bridge CFTypeRef)self.window };
    
    CUIControllerSetCredUIContext(_controller, kCUICredUIContextPropertyParentWindow, &uic);
    
    self.credsController = [[NSArrayController alloc] init];
    self.credsController.selectsInsertedObjects = NO;
    
    [self.collectionView bind:NSContentBinding toObject:self.credsController withKeyPath:@"arrangedObjects" options:nil];
    [self.collectionView bind:NSSelectionIndexesBinding toObject:self.credsController withKeyPath:@"selectionIndexes" options:nil];
    
    CUIControllerEnumerateCredentials(_controller, ^(CUICredentialRef cred, CFErrorRef error) {
        if (cred)
            [self.credsController addObject:(__bridge CUICredential *)cred];
        else if (error)
            NSLog(@"CUIControllerEnumerateCredentials: %@", error);
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
    __block NSModalResponse modalResponse;
    
    if (self.title)
        self.window.title = self.title;
    if (self.message)
        self.messageTextField.stringValue = self.message;
    
    [self _populateCredentials];
    
    if (window) {
        [window beginSheet:self.window completionHandler:^(NSModalResponse sheetReturnCode) {
            modalResponse = sheetReturnCode;
        }];
    } else {
        NSModalSession modalSession = [NSApp beginModalSessionForWindow:self.window];
        do {
            modalResponse = [NSApp runModalSession:modalSession];
            [[NSRunLoop currentRunLoop] limitDateForMode:NSDefaultRunLoopMode];
            if (self.autoLogin) {
                [self willSubmitCredential:self.submitButton];
                self.autoLogin = NO;
            }
        } while (modalResponse == NSModalResponseContinue);
        
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
        [self.selectedCredential didConfirm:NULL];
}

- (void)credentialFieldDidChange:(id)sender
{
    CUICredentialTile *tile = (CUICredentialTile *)[sender superview];
    
    if ([tile.credential isEqual:self.selectedCredential])
        [self _updateSubmitButtonForSelectedCred];
}

- (void)didClickPersist:(id)sender
{
    self.persist = ((NSButton *)sender).state;
}

#pragma mark - Accessors

- (NSString *)title
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    return (__bridge NSString *)uic->titleText;
}

- (void)setTitle:(NSString *)aTitle
{
    CUICredUIContext uic = { .version = 0, .titleText = (__bridge CFStringRef)aTitle };
    CUIControllerSetCredUIContext(_controller, kCUICredUIContextPropertyTitleText, &uic);
}

- (NSString *)message
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    return (__bridge NSString *)uic->messageText;
}

- (void)setMessage:(NSString *)aMessage
{
    CUICredUIContext uic = { .version = 0, .titleText = (__bridge CFStringRef)aMessage };
    CUIControllerSetCredUIContext(_controller, kCUICredUIContextPropertyMessageText, &uic);
}

- (CUICredUIContext *)credUIContext
{
    return (CUICredUIContext *)CUIControllerGetCredUIContext(_controller);
}

- (void)setCredUIContext:(CUICredUIContext *)uic
{
    CUIControllerSetCredUIContext(_controller, kCUICredUIContextPropertyAll, uic);
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
    return (__bridge GSSContext *)CUIControllerGetGssContextHandle(_controller);
}

- (void)setGSSContextHandle:(GSSContext *)aContext
{
    CUIControllerSetGssContextHandle(_controller, (__bridge CFTypeRef)aContext);
}

- (id)targetName
{
    return (__bridge id)CUIControllerGetTargetName(_controller);
}

- (void)setTargetName:(id)aTarget
{
    CFTypeRef cfTarget = (__bridge CFTypeRef)aTarget;
    
    if ((self.flags & CUIFlagsGenericCredentials) == 0 &&
        CFGetTypeID(cfTarget) == CFStringGetTypeID()) {
        CFErrorRef error = NULL;
        gss_name_t gssName = GSSCreateName(cfTarget, GSS_C_NT_HOSTBASED_SERVICE, &error);
        
        if (gssName) {
            CUIControllerSetTargetName(_controller, gssName);
            CFRelease(gssName);
        } else if (error) {
            CFRelease(error);
        }
    } else {
        CUIControllerSetTargetName(_controller, cfTarget);
    }
}

- (CUICredential *)selectedCredential
{
    NSArray *selectedObjects;
    CUICredential *cred = nil;
        
    selectedObjects = [self.credsController selectedObjects];
    if (selectedObjects.count)
        cred = selectedObjects[0];
    
    return cred;
}

@end
