//
//  CUIIdentityPickerInternalWindowController.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

Boolean
_CUIIsReturnableCredentialStatus(CFTypeRef status, Boolean *);

@interface CUIIdentityPickerInternal ()

@property(nonatomic, assign) CUIControllerRef controllerRef;
@property(nonatomic, assign) CUICredUIContext *credUIContext;
@property(nonatomic, assign) CUIFlags flags;

@property(nonatomic, assign) BOOL runningModal;

@property(nonatomic, retain, readonly) NSString *targetHostName;

+ (CUIControllerRef)_newCUIController:(CUIUsageScenario)usageScenario flags:(CUIFlags)flags;
- (BOOL)_loadViews;

@end

@implementation CUIIdentityPickerInternal

@synthesize persist = _persist;
@synthesize flags = _flags;
@synthesize lastError = _lastError;
@synthesize contextBox = _contextBox;

@synthesize collectionView = _collectionView;
@synthesize titleTextField = _titleTextField;
@synthesize messageTextField = _messageTextField;
@synthesize persistCheckBox = _persistCheckBox;
@synthesize submitButton = _submitButton;
@synthesize cancelButton = _cancelButton;

@synthesize credUIContext = _credUIContext;
@synthesize credsController = _credsController;
@synthesize runningModal = _runningModal;

- (void)dealloc
{
    if (_controllerRef)
        CFRelease(_controllerRef);
    
#if !__has_feature(objc_arc)
    [_lastError release];
    [_contextBox release];
    [super dealloc];
#endif
}

- (CUIControllerRef)controllerRef
{
    return _controllerRef;
}

- (void)setControllerRef:(CUIControllerRef)controllerRef
{
    if (controllerRef != _controllerRef) {
        if (_controllerRef)
            CFRelease(_controllerRef);
        _controllerRef = (CUIControllerRef)CFRetain(controllerRef);
    }
}

+ (CUIControllerRef)_newCUIController:(CUIUsageScenario)usageScenario
                                flags:(CUIFlags)flags
{
    CUIUsageFlags usageFlags = 0;
    
    if (flags & CUIFlagsGenericCredentials)
        usageFlags |= kCUIUsageFlagsGeneric;
    if (flags & CUIFlagsExcludeCertificates)
        usageFlags |= kCUIUsageFlagsExcludeCertificates;
    if (flags & (CUIFlagsRequireCertificate | CUIFlagsRequireSmartcard))
        usageFlags |= kCUIUsageFlagsRequireCertificates;
    if (flags & CUIFlagsPasswordOnlyOK)
        usageFlags |= kCUIUsageFlagsPasswordOnlyOK;
    if (flags & CUIFlagsKeepUsername)
        usageFlags |= kCUIUsageFlagsKeepUsername;
    if (flags & CUIFlagsExcludePersistedCredentials)
        usageFlags |= kCUIUsageFlagsExcludePersistedCreds;
    if (flags & CUIFlagsExcludeTransientCredentials)
        usageFlags |= kCUIUsageFlagsExcludeTransientCreds;
    
    return CUIControllerCreate(kCFAllocatorDefault, usageScenario, usageFlags);
}

- (BOOL)isConfigured
{
    return (self.controllerRef != NULL);
}

- (BOOL)configureForUsageScenario:(CUIUsageScenario)usageScenario
                            flags:(CUIFlags)flags
{
    CUIControllerRef controllerRef;

    NSAssert(self.runningModal == NO, @"cannot configure identity picker during run loop");

    if (usageScenario != kCUIUsageScenarioNetwork) {
        /* Make sure login credentails can never be persisted */
        flags &= ~(CUIFlagsPersist);
        flags |= CUIFlagsDoNotPersist;
    }

    controllerRef = [CUIIdentityPickerInternal _newCUIController:usageScenario flags:flags];
    if (controllerRef == nil) {
        NSLog(@"Failed to initialize CUIController");
        return NO;
    }

    self.controllerRef = controllerRef;
    CFRelease(controllerRef);
 
    self.flags = flags;
    if ((self.flags & (CUIFlagsPersist | CUIFlagsDoNotPersist)) == 0)
        self.flags |= CUIFlagsShowSaveCheckBox;

    self.persist = !!(self.flags & CUIFlagsPersist); 

    return YES;
}

- init
{
    self = [super init];
    if (self == nil)
        return nil;

    if (![self _loadViews]) {
#if !__has_feature(objc_arc)
        [self release];
#endif
        return nil;
    }
  
    return self;
}

- (BOOL)_loadViews
{
    BOOL bLoaded;
    NSArray *objects = nil;
    
    bLoaded = [[NSBundle credUIBundle] loadNibNamed:@"CUIIdentityPicker" owner:self topLevelObjects:&objects];
    NSAssert(bLoaded, @"Could not load identity picker nib");
    
    if (!bLoaded)
        return NO;
    
    if ((self.flags & CUIFlagsShowSaveCheckBox) == 0)
        [self.persistCheckBox setHidden:YES];
   
    return YES;
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
       
        self.submitButton.enabled = NO;
        
        for (index = 0; index < creds.count; index++) {
            CUICredential *cred = [creds objectAtIndex:index];
            
            if ([indexes containsIndex:index]) {
                [self observeCredential:cred];
                [cred didBecomeSelected];
            } else {
                [cred didBecomeDeselected];
                if ([cred observationInfo])
                    [self unobserveCredential:cred];
            }
        }
    } else if ([keyPath isEqualTo:(__bridge NSString *)kCUIAttrCredentialStatus]) {
        id credStatus = [change objectForKey:NSKeyValueChangeNewKey];
        Boolean autoLogin;

        self.submitButton.enabled = _CUIIsReturnableCredentialStatus((__bridge CFTypeRef)credStatus, &autoLogin);

        if (autoLogin)
            [self willSubmitCredential:self.submitButton];
    }
}

- (void)startCredentialEnumeration:(NSWindow *)parentWindow
{
    NSString *targetHostName;
   
    NSAssert(self.isConfigured, @"must configure identity picker before use");
    NSAssert(!self.runningModal, @"only one identity picker instance can run at a time");
    
    if (self.title)
        self.titleTextField.stringValue = self.title;
    else if ((targetHostName = self.targetHostName))
        self.titleTextField.stringValue = [NSString stringWithFormat:@"Connecting to %@", targetHostName];
    if (self.message)
        self.messageTextField.stringValue = self.message;
    else
        self.messageTextField.stringValue = @"Enter your credentials";

    CUICredUIContext uic = { .version = 0, .parentWindow = (__bridge CFTypeRef)parentWindow };
    [self setCredUIContext:&uic properties:kCUICredUIContextPropertyParentWindow];

    CUIControllerSetContext(self.controllerRef, self.contextBox.context);

    self.credsController.selectsInsertedObjects = NO;

    CUIControllerEnumerateCredentials(self.controllerRef, ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef error) {
        if (cred) {
            [self.credsController addObject:(__bridge CUICredential *)cred];
            if (isDefault)
                self.credsController.selectionIndex = [self.credsController.arrangedObjects count] - 1;
        } else if (error) {
            NSLog(@"CUIControllerEnumerateCredentials: %@", error);
            self.lastError = (__bridge NSError *)error;
        }
    });
    
    if (self.credsController.selectionIndex == NSNotFound)
        self.credsController.selectionIndex = 0;

    [self.collectionView addObserver:self
                          forKeyPath:@"selectionIndexes"
                             options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                             context:NULL];

    /* Generic credentials can be automatically submitted if there is an available credential. */
    if ((self.flags & CUIFlagsGenericCredentials) &&
        (self.flags & CUIFlagsAlwaysShowUI) == 0 &&
        [self.selectedCredential canSubmit])
        [self willSubmitCredential:self.submitButton];

    self.runningModal = YES;
}

- (void)endCredentialEnumeration:(NSModalResponse)modalResponse
{
    self.runningModal = NO;

    if (modalResponse == NSModalResponseOK) {
        /* Propagate asynchronous errors from credential */
        NSError *credError = [self.selectedCredential.attributes objectForKey:(__bridge id)kCUIAttrCredentialError];

        if (credError) {
            self.lastError = credError;
        } else {
            [self didSubmitCredential];
        }
    }

    [self.collectionView removeObserver:self forKeyPath:@"selectionIndexes"];
}    

#pragma mark - Run Loop

- (NSInteger)runModal
{
    NSModalResponse modalResponse;

    [self startCredentialEnumeration:self.window];

    modalResponse = [NSApp runModalForWindow:self.window];

    [self endCredentialEnumeration:modalResponse];

    return modalResponse;
}

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow
               completionHandler:(void (^)(NSModalResponse returnCode))handler
{
    [self startCredentialEnumeration:self.window];

    [sheetWindow beginSheet:self.window
     completionHandler:^(NSModalResponse returnCode) {
        [self endCredentialEnumeration:returnCode];
        handler(returnCode);
    }];
}

#pragma mark - Credential submission

#pragma mark Observation

- (void)observeCredential:(CUICredential *)credential
{
    [credential.attributes addObserver:self
                            forKeyPath:(__bridge NSString *)kCUIAttrCredentialStatus
                               options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                               context:NULL];
}

- (void)unobserveCredential:(CUICredential *)credential
{
    [credential.attributes removeObserver:self
                               forKeyPath:(__bridge NSString *)kCUIAttrCredentialStatus];
}

#pragma mark Cancel submission

- (void)endWithReturnCode:(NSModalResponse)returnCode
                   sender:(id)sender
{
    if (self.window.isSheet) {
        [self.window.sheetParent endSheet:self.window returnCode:returnCode];
    } else {
        [NSApp stopModalWithCode:returnCode];
    }
    [self.window orderOut:sender];
}

- (void)willCancelCredential:(id)sender
{
    [self endWithReturnCode:NSModalResponseCancel sender:sender];
}
 
- (IBAction)didClickCancel:(id)sender
{
    [self willCancelCredential:sender];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [self willCancelCredential:nil];
}

#pragma mark OK submission

- (void)willSubmitCredential:(id)sender
{
    self.submitButton.state = NSOnState; // in case cred was submitted without clicking

    [self.selectedCredential willSubmit];
    [self endWithReturnCode:NSModalResponseOK sender:sender];
}

- (IBAction)didClickOK:(id)sender
{
    [self willSubmitCredential:sender];
}

- (void)didSubmitCredential
{
    [self.selectedCredential didSubmit];
    
    if (self.persist &&
        (self.flags & CUIFlagsExpectConfirmation) == 0)
        [self.selectedCredential savePersisted:^(NSError *error) {
        }];
}

#pragma mark Persist

- (IBAction)didClickPersist:(id)sender
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

