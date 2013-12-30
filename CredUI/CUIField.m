//
//  CUIField.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//


@implementation CUIField
{
}

#pragma mark Initialization

+ (void)load
{
    _CFRuntimeBridgeClasses(CUIFieldGetTypeID(), "CUIField");
}

+ (id)allocWithZone:(NSZone *)zone
{
    return nil;
}

#pragma mark Bridging

- (id)retain
{
    return CFRetain((CFTypeRef)self);
}

- (oneway void)release
{
    CFRelease((CFTypeRef)self);
}

- (NSUInteger)retainCount
{
    return CFGetRetainCount((CFTypeRef)self);
}

- (BOOL)isEqual:(id)anObject
{
    return (BOOL)CFEqual((CFTypeRef)self, (CFTypeRef)anObject);
}

- (NSUInteger)hash
{
    return CFHash((CFTypeRef)self);
}

- (NSString *)description
{
    CFStringRef copyDesc = CFCopyDescription((CFTypeRef)self);
    
    return CFBridgingRelease(copyDesc);
}

- (BOOL)allowsWeakReference
{
    return !_CFIsDeallocating(self);
}

- (BOOL)retainWeakReference
{
    return _CFTryRetain(self) != nil;
}

- (CFTypeID)_cfTypeID
{
    return CUIFieldGetTypeID();
}

- initWithClass:(CUIFieldClass)fieldClass
          title:(NSString *)title
   defaultValue:(id)defaultValue
       delegate:(void(^)(CUIFieldRef, CFTypeRef))fieldDidChange
{
    CUIFieldRef fieldRef;
    
    fieldRef = CUIFieldCreate(kCFAllocatorDefault,
                              fieldClass,
                              (__bridge CFStringRef)title,
                              (__bridge CFTypeRef)defaultValue,
                              fieldDidChange);
    
    self = (__bridge id)fieldRef;
    
    return self;
}

- init
{
    return [self initWithClass:kCUIFieldClassInvalid
                         title:nil
                  defaultValue:nil
                      delegate:nil];
}

- (CUIFieldRef)_fieldRef
{
    return (__bridge CUIFieldRef)self;
}

- (CUIFieldClass)fieldClass
{
    return CUIFieldGetClass([self _fieldRef]);
}

- (NSString *)title
{
    return (__bridge NSString *)CUIFieldGetTitle([self _fieldRef]);
}

- (id)defaultValue
{
    return (__bridge id)CUIFieldGetDefaultValue([self _fieldRef]);
}

- (void)setValue:(id)aValue
{
    CUIFieldSetValue([self _fieldRef], (__bridge CFTypeRef)aValue);
}

- (void)didSubmit:(id)sender
{
    CUIIdentityPicker *picker = [[sender window] delegate];
    CUICredentialTile *tile = (CUICredentialTile *)[sender superview];
    
    [self setValue:(__bridge id)kCFBooleanTrue];
    [picker _selectCredential:[tile credential]];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextView *textView = notification.userInfo[@"NSFieldEditor"];
    [self setValue:textView.string];
}

@end
