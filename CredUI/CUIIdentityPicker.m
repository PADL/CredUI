//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIIdentityPicker
{
    CUIFlags _flags;
    CUIControllerRef _controller;
}

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
    return [self initWithFlags:flags authIdentity:nil];
}

- initWithFlags:(CUIFlags)flags authIdentity:(NSDictionary *)authIdentity
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
    
    if (authIdentity)
        self.authIdentity = authIdentity;
    
    return self;
}

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

- (NSDictionary *)authIdentity
{
    return (__bridge NSDictionary *)CUIControllerGetAuthIdentity(_controller);
}

- (void)setAuthIdentity:(NSDictionary *)anAuthIdentity
{
    CUIControllerSetDefaultAuthIdentity(_controller, (__bridge CFDictionaryRef)anAuthIdentity);
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

- (BOOL)_enumerateCredentials:(void (^)(CUICredentialRef))cb
{
    return CUIControllerEnumerateCredentials(_controller, cb);
}

- (NSInteger)runModal
{
    return NSCancelButton;
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
}
@end