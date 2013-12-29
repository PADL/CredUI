//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "CUIIdentityPicker.h"

@interface CUIIdentityPicker () <NSWindowDelegate>
@property (nonatomic, retain) NSMutableArray *creds;
@end

@implementation CUIIdentityPicker
{
    CUIFlags _flags;
    CUIControllerRef _controller;
    NSPanel *_panel;
}

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

- initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes
{
    CUIUsageFlags usageFlags = 0;
    
    if ((self = [super init]) == nil)
        return nil;
    
    if (flags & CUIShowSaveCheckBox)
        usageFlags |= kCUIUsageFlagsSaveCheckbox;
    if (flags & CUIFlagsGenericCredentials)
        usageFlags |= kCUIUsageFlagsGeneric;
    
    _controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, usageFlags);
    if (_controller == NULL)
        return nil;
    
    if (attributes)
        self.attributes = attributes;

    NSRect frame = NSMakeRect(0, 0, 400, 600);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask | NSUtilityWindowMask;
    NSRect rect = [NSPanel contentRectForFrameRect:frame styleMask:styleMask];

    _panel = [[NSPanel alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];
    _panel.hidesOnDeactivate = YES;
    _panel.worksWhenModal = YES;
    _panel.delegate = self;
    
    return self;
}

- (void)_preparedToEnumerateCredentials
{
    const CUICredUIContext *uic = CUIControllerGetCredUIContext(_controller);
    CUICredUIContext newUic = *uic;
    
    newUic.parentWindow = (__bridge CFTypeRef)_panel;
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
    
    [self _preparedToEnumerateCredentials]; // fixes up parent window
    if (![self _enumerateCredentials])
        return NSModalResponseStop;
    
    collectionView = [[NSCollectionView alloc] initWithFrame:[[_panel contentView] frame]];
    collectionView.itemPrototype = [[CUICredentialTileController alloc] init];
    collectionView.content = _creds;
    collectionView.selectable = TRUE;
    collectionView.autoresizingMask = (NSViewMinXMargin
                                         | NSViewWidthSizable
                                         | NSViewMaxXMargin
                                         | NSViewMinYMargin
                                         | NSViewHeightSizable
                                         | NSViewMaxYMargin);

    [_panel.contentView addSubview:collectionView];    

    return [NSApp runModalForWindow:_panel];
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    NSInteger returnCode = [self runModal];
    NSMethodSignature *signature = [delegate methodSignatureForSelector:didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *selfPtr = (__bridge void *)self;
    
    [invocation setTarget:delegate];
    [invocation setSelector:didEndSelector];
    [invocation setArgument:&selfPtr atIndex:2];
    [invocation setArgument:&returnCode atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
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

@synthesize flags = _flags;

- (GSSContext *)contextHandle
{
    return (__bridge GSSContext *)CUIControllerGetGssContextHandle(_controller);
}

- (void)setGssContextHandle:(GSSContext *)aContext
{
    CUIControllerSetGssContextHandle(_controller, (__bridge CFTypeRef)aContext);
}

- (id)target
{
    return (__bridge id)CUIControllerGetGssTargetName(_controller);
}

- (void)setTarget:(id)aTarget
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
@end