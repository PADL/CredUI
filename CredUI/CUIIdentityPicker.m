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
@property (nonatomic, retain) NSMutableArray *creds;
@property (nonatomic, retain) NSPanel *panel;
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
    NSRect frame = NSMakeRect(0, 0, 400, 600);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSUtilityWindowMask;
    NSRect rect = [NSPanel contentRectForFrameRect:frame styleMask:styleMask];
    NSPanel *panel = [[NSPanel alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];
    panel.hidesOnDeactivate = YES;
    panel.worksWhenModal = YES;
    panel.delegate = self;

    return panel;
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

    return self;
}

- (void)_prepareToEnumerateCredentials
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    CUICredUIContext newUic = *uic;
    
    newUic.parentWindow = (__bridge CFTypeRef)self.panel;
    CUIControllerSetCredUIContext(_controller, &newUic);
}

- (BOOL)_enumerateCredentials
{
    _creds = [[NSMutableArray alloc] init];
    
    return CUIControllerEnumerateCredentials(_controller, ^(CUICredentialRef cred) {
        [_creds addObject:(__bridge id)cred];
    });
}

- (void)windowWillClose:(NSNotification *)notification
{
    [NSApp stopModalWithCode:NSModalResponseStop];
}

- (NSInteger)runModal
{
    NSCollectionView *collectionView;
    
    [self _prepareToEnumerateCredentials]; // fixes up parent window
    if (![self _enumerateCredentials])
        return NSModalResponseStop;
    
    self.panel = [self _newPanel];

    collectionView = [[NSCollectionView alloc] initWithFrame:[[self.panel contentView] frame]];
    collectionView.itemPrototype = [[CUICredentialTileController alloc] init];
    collectionView.content = _creds;
    collectionView.selectable = TRUE;
    collectionView.autoresizingMask = (NSViewMinXMargin
                                         | NSViewWidthSizable
                                         | NSViewMaxXMargin
                                         | NSViewMinYMargin
                                         | NSViewHeightSizable
                                         | NSViewMaxYMargin);

    [self.panel.contentView addSubview:collectionView];

    NSInteger returnCode = [NSApp runModalForWindow:self.panel];
    if (returnCode == NSModalResponseStop) {
        NSIndexSet *indices = [collectionView selectionIndexes];
        NSUInteger firstIndex = [indices firstIndex];
        
        if (firstIndex != NSNotFound)
            self.selectedCredential = [_creds objectAtIndex:firstIndex];
    }

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
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    CUICredUIContext newUic = *uic;
    
    newUic.titleText = (__bridge CFStringRef)aTitle;
    CUIControllerSetCredUIContext(_controller, &newUic);
}

- (NSString *)message
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    return (__bridge NSString *)uic->messageText;
}

- (void)setMessage:(NSString *)aMessage
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    CUICredUIContext newUic = *uic;
    
    newUic.messageText = (__bridge CFStringRef)aMessage;
    CUIControllerSetCredUIContext(_controller, &newUic);
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