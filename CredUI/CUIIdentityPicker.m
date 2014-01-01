//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIIdentityPicker
#pragma mark - Implementation

- (void)dealloc
{
    if (_controller)
        CFRelease(_controller);
}

- (NSPanel *)_newPanel
{
    NSRect frame = NSMakeRect(0, 0, 400, 450);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask;
    NSRect rect = [NSPanel contentRectForFrameRect:frame styleMask:styleMask];
    NSPanel *panel = [[NSPanel alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];
    panel.hidesOnDeactivate = YES;
    panel.worksWhenModal = YES;
    panel.delegate = self;

    return panel;
}

- (NSCollectionView *)_newCollectionViewWithPanel:(NSPanel *)panel
{
    NSCollectionView *collectionView;
    
    collectionView = [[NSCollectionView alloc] initWithFrame:[[panel contentView] frame]];
    collectionView.itemPrototype = [[CUICredentialTileController alloc] init];
    collectionView.selectable = YES;
    collectionView.allowsMultipleSelection = NO;
    collectionView.autoresizingMask = (NSViewMinXMargin
                                       | NSViewWidthSizable
                                       | NSViewMaxXMargin
                                       | NSViewMinYMargin
                                       | NSViewHeightSizable
                                       | NSViewMaxYMargin);
    collectionView.autoresizesSubviews = YES;
    
    return collectionView;
}

- (NSTextField *)_newMessageTextField
{
    NSRect frame = NSMakeRect(0, 0, 400, 50);
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    textField.editable = NO;
    textField.selectable = NO;
    textField.bordered = YES;
    textField.backgroundColor = [NSColor lightGrayColor];
    
    return textField;
}

- (NSButton *)_newSubmitButton
{
    NSRect frame = NSMakeRect(0, 0, 400, 30);
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = @"OK";
    button.target = self;
    button.action = @selector(willSubmitCredential:);
    
    return button;
}

- (instancetype)init
{
    return [self initWithFlags:0];
}

- (instancetype)initWithFlags:(CUIFlags)flags
{
    return [self initWithFlags:flags attributes:nil];
}

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes
{
    CUIUsageFlags usageFlags = 0;
    
    if ((self = [super init]) == nil)
        return nil;
    
    self.flags = flags;
    
    if (self.flags & CUIShowSaveCheckBox)
        usageFlags |= kCUIUsageFlagsSaveCheckbox;
    if (self.flags & CUIFlagsGenericCredentials)
        usageFlags |= kCUIUsageFlagsGeneric;
    if (self.flags & CUIFlagsExcludePersistedCredentials)
        usageFlags |= kCUIUsageFlagsExcludePersistedCreds;
    
    _controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, usageFlags);
    if (_controller == NULL)
        return nil;
    
    if (attributes)
        self.attributes = attributes;
    
    self.panel = [self _newPanel];

    self.messageTextField = [self _newMessageTextField];
    [self.panel.contentView addSubview:self.messageTextField];
    
    self.collectionView = [self _newCollectionViewWithPanel:self.panel];
    [self.panel.contentView addSubview:self.collectionView];
    
    [self.panel.contentView addSubview:[self _newSubmitButton]];
    
    return self;
}

- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp stopModalWithCode:NSModalResponseStop];
}

-(void)observeValueForKeyPath:(NSString *)keyPath
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
                [self willSubmitCredential:nil];
            }
        }
    }
}

- (void)_populateCredentials
{
    CUICredUIContext uic = { .version = 0, .parentWindow = (__bridge CFTypeRef)self.panel };
    
    CUIControllerSetCredUIContext(_controller, kCUICredUIContextPropertyParentWindow, &uic);
    
    self.credsController = [[NSArrayController alloc] init];
    self.credsController.avoidsEmptySelection = NO;
    self.credsController.selectsInsertedObjects = NO;
    
    [self.collectionView bind:NSContentBinding toObject:self.credsController withKeyPath:@"arrangedObjects" options:nil];
    [self.collectionView bind:NSSelectionIndexesBinding toObject:self.credsController withKeyPath:@"selectionIndexes" options:nil];

    CUIControllerEnumerateCredentials(_controller, ^(CUICredentialRef cred, CFErrorRef error) {
        if (cred)
            [self.credsController addObject:(__bridge CUICredential *)cred];
        else if (error)
            NSLog(@"CUIControllerEnumerateCredentials: %@", error);
    });
    
    [self.collectionView addObserver:self
                          forKeyPath:@"selectionIndexes"
                             options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                             context:nil];
    
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    *((NSInteger *)contextInfo) = returnCode;
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    __block NSModalResponse returnCode;
    
    if (self.title)
        self.panel.title = self.title;
    if (self.message)
        self.messageTextField.stringValue = self.message;
    
    [self _populateCredentials];
    
    if (window) {
        [window beginSheet:self.panel completionHandler:^(NSModalResponse sheetReturnCode) {
            returnCode = sheetReturnCode;
        }];
    } else {
        returnCode = [NSApp runModalForWindow:self.panel];
    }
    if (returnCode == NSModalResponseStop) {
        NSArray *selectedObjects = [self.credsController selectedObjects];
        
        if (selectedObjects.count)
            self.selectedCredential = selectedObjects[0];
    }
    
    [self.collectionView removeObserver:self forKeyPath:@"selectionIndexes"];
    [self.credsController unbind:NSContentBinding];
    [self.credsController unbind:NSSelectionIndexesBinding];
    
    // do this outside modal loop
    [self didSubmitCredential];
    
    NSMethodSignature *signature = [delegate methodSignatureForSelector:didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *object = (__bridge void *)self;
    
    [invocation setTarget:delegate];
    [invocation setSelector:didEndSelector];
    [invocation setArgument:&object atIndex:2];
    [invocation setArgument:&returnCode atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
}

- (NSInteger)runModal
{
    NSInteger returnCode;
    
    [self runModalForWindow:nil
              modalDelegate:self
             didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                contextInfo:&returnCode];
    
    return returnCode;
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
    return (__bridge id)CUIControllerGetGssTargetName(_controller);
}

- (void)setTargetName:(id)aTarget
{
    CFTypeRef cfTarget = (__bridge CFTypeRef)aTarget;
    
    if (CFGetTypeID(cfTarget) == CFStringGetTypeID()) {
        CFErrorRef error = NULL;
        gss_name_t gssName = GSSCreateName(cfTarget, GSS_C_NT_HOSTBASED_SERVICE, &error);
        
        if (gssName) {
            CUIControllerSetGssTargetName(_controller, gssName);
            CFRelease(gssName);
        } else if (error) {
            CFRelease(error);
        }
    } else {
        CUIControllerSetGssTargetName(_controller, (gss_name_t)cfTarget);
    }
}

- (NSString *)targetDisplayName
{
    if (self.targetName == nil)
        return NULL;
    
    CFStringRef displayName = GSSNameCreateDisplayString((__bridge gss_name_t)self.targetName);
    
    return CFBridgingRelease(displayName);
}

- (CUICredential *)selectedCredential
{
    NSArray *selectedObjects = [self.credsController selectedObjects];
    CUICredential *cred = nil;
    
    if (selectedObjects.count)
        cred = selectedObjects[0];
    
    return cred;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p{targetName = \"%@\", selectedCredential = %@, flags = 0x%08x}>",
            [self.class description],
            self,
            self.targetDisplayName,
            self.selectedCredential,
            (unsigned int)self.flags];
}

#pragma mark - Credential submission

- (void)willSubmitCredential:(id)sender
{
    [self.selectedCredential willSubmit];

    if ([self.selectedCredential canSubmit])
        [self.panel close];
}

- (void)didSubmitCredential
{
    NSError *error;
    
    [self.selectedCredential didSubmit];
    
    if (self.saveToKeychain && self.selectedCredential.GSSItem == nil)
        [self.selectedCredential addGSSItem:&error];
    
}

@end