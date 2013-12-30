//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPicker () <NSWindowDelegate>
@property (nonatomic, assign) CUIFlags flags;
@property (nonatomic, assign) CUIControllerRef controller;
@property (nonatomic, retain) id selectedCredential;
@property (nonatomic, retain) NSPanel *panel;
@property (nonatomic, retain) NSArrayController *credsController;
@property (nonatomic, retain) NSCollectionView *collectionView;
@end

@implementation CUIIdentityPicker
#pragma mark - Implementation

- (void)dealloc
{
    
    if (_controller)
        CFRelease(_controller);
}

- init
{
    return [self initWithFlags:0];
}

- initWithFlags:(CUIFlags)flags
{
    return [self initWithFlags:flags attributes:nil];
}

- (NSPanel *)_newPanel
{
    NSRect frame = NSMakeRect(0, 0, 400, 450);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSUtilityWindowMask;
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

- initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes
{
    CUIUsageFlags usageFlags = 0;
    
    if ((self = [super init]) == nil)
        return nil;
    
    self.flags = flags;
    
    if (self.flags & CUIShowSaveCheckBox)
        usageFlags |= kCUIUsageFlagsSaveCheckbox;
    if (self.flags & CUIFlagsGenericCredentials)
        usageFlags |= kCUIUsageFlagsGeneric;
    
    _controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, usageFlags);
    if (_controller == NULL)
        return nil;
    
    if (attributes)
        self.attributes = attributes;
    
    self.panel = [self _newPanel];
    self.collectionView = [self _newCollectionViewWithPanel:self.panel];
    [self.panel.contentView addSubview:self.collectionView];
    
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
            CUICredentialRef credRef = (__bridge CUICredentialRef)[creds objectAtIndex:index];
            Boolean autoLogin = false;
            
            NSLog(@"cred %@ selected %d", credRef, [indexes containsIndex:index]);
            
            if ([indexes containsIndex:index])
                CUICredentialDidBecomeSelected(credRef, &autoLogin);
            else
                CUICredentialDidBecomeDeselected(credRef);

            if (autoLogin)
                [NSApp stopModalWithCode:NSModalResponseStop];
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

    CUIControllerEnumerateCredentials(_controller, ^(CUICredentialRef cred) {
        [self.credsController addObject:(__bridge id)cred];
    });
    
    [self.collectionView addObserver:self
                          forKeyPath:@"selectionIndexes"
                             options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                             context:nil];
    
}

- (NSInteger)runModal
{
    [self _populateCredentials];
    
    NSInteger returnCode = [NSApp runModalForWindow:self.panel];

    if (returnCode == NSModalResponseStop) {
        NSArray *selectedObjects = [self.credsController selectedObjects];
        
        if (selectedObjects.count)
            self.selectedCredential = selectedObjects[0];
    }

    [self.collectionView removeObserver:self forKeyPath:@"selectionIndexes"];
    [self.credsController unbind:NSContentBinding];
    [self.credsController unbind:NSSelectionIndexesBinding];

    return returnCode;
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    NSInteger returnCode = [self runModal];
    NSMethodSignature *signature = [delegate methodSignatureForSelector:didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *object = (__bridge void *)self;
    
    [invocation setTarget:delegate];
    [invocation setSelector:didEndSelector];
    [invocation setArgument:&object atIndex:2];
    [invocation setArgument:&returnCode atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
    
    if (object)
        CFRelease(object);
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

- (NSDictionary *)attributes
{
    return (__bridge NSDictionary *)CUIControllerGetAttributes(_controller);
}

- (void)setAttributes:(NSDictionary *)someAttributes
{
    CUIControllerSetAttributes(_controller, (__bridge CFDictionaryRef)someAttributes	);
}

- (BOOL)saveToKeychain
{
    return CUIControllerGetSaveToKeychain(_controller);
}

- (void)setSaveToKeychain:(BOOL)save
{
    CUIControllerSetSaveToKeychain(_controller, save);
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
        gss_name_t gssName = GSSCreateName(cfTarget, GSS_C_NT_USER_NAME, &error);
        
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

- (CUICredentialRef)selectedCredentialRef
{
    return (__bridge CUICredentialRef)self.selectedCredential;
}

- (NSDictionary *)selectedCredentialAttributes
{
    if (self.selectedCredentialRef == NULL)
        return nil;
    
    return (__bridge NSDictionary *)CUICredentialGetAttributes(self.selectedCredentialRef);
}

- (__autoreleasing GSSItem *)selectedGSSItem:(NSError * __autoreleasing *)error
{
    GSSItemRef itemRef;
    CFErrorRef cfError = NULL;

    if (error != NULL)
        *error = nil;
    
    if (self.selectedCredentialRef == NULL)
        return nil;
    
    itemRef = CUICredentialCreateGSSItem(self.selectedCredentialRef, true, &cfError);
    if (cfError) {
        if (error)
            *error = CFBridgingRelease(cfError);
        else
            CFRelease(cfError);
    }
    
    return CFBridgingRelease(itemRef);
}

@end