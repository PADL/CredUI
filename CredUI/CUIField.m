//
//  CUIField.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CFBridgeHelper.h"

@interface CUICFField : CUIField
@end

@implementation CUICFField

+ (void)load
{
    _CFRuntimeBridgeClasses(CUIFieldGetTypeID(), "CUICFField");
}

+ (id)allocWithZone:(NSZone *)zone
{
    static CUICFField *placeholderField;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (placeholderField == nil)
            placeholderField = [super allocWithZone:zone];
    });

    return placeholderField;
}

CF_CLASSIMPLEMENTATION(CUICFField)

- initWithClass:(CUIFieldClass)fieldClass
          title:(NSString *)title
   defaultValue:(id)defaultValue
       delegate:(void(^)(CUIFieldRef, CFTypeRef))fieldDidChange
{
    CUIFieldRef fieldRef;
    NSString *titleCopy = [title copy];
    id defaultValueCopy = [defaultValue copy];
    
    fieldRef = CUIFieldCreate(kCFAllocatorDefault,
                              fieldClass,
                              (CFStringRef)titleCopy,
                              (CFTypeRef)defaultValueCopy,
                              fieldDidChange);
    
    [titleCopy release];
    [defaultValueCopy release];
        
    self = (id)fieldRef;
    
    return NSMakeCollectable(self);
}

@end

@implementation CUIField

- (CUIFieldRef)_fieldRef
{
    return (CUIFieldRef)self;
}

- (NSString *)description
{
    return [NSMakeCollectable(CFCopyDescription([self _fieldRef])) autorelease];
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
    self = [super init];
    
    return self;
}

- init
{
    return [self initWithClass:kCUIFieldClassInvalid
                         title:nil
                  defaultValue:nil
                      delegate:nil];
}

- (CUIFieldClass)fieldClass
{
    return CUIFieldGetClass([self _fieldRef]);
}

- (NSString *)title
{
    return (NSString *)CUIFieldGetTitle([self _fieldRef]);
}

- (id)defaultValue
{
    return (id)CUIFieldGetDefaultValue([self _fieldRef]);
}

- (void)setValue:(id)aValue
{
    id aValueCopy = [aValue copy];
    
    CUIFieldSetValue([self _fieldRef], (CFTypeRef)aValueCopy);
    
    [aValueCopy release];
}

- (void)setValue:(id)aValue sender:(id)sender
{
    id identityPicker = [[(NSView *)sender window] delegate];

    [self setValue:aValue];
    [identityPicker credentialFieldDidChange:sender];
}

- (void)didSubmit:(id)sender
{
    [self setValue:(id)kCFBooleanTrue];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextView *textView = notification.userInfo[@"NSFieldEditor"];
    
    [self setValue:textView.string sender:notification.object];
}

- (BOOL)isUsernameField
{
    return [self.title isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSUsername];
}

- (BOOL)isPasswordField
{
    return [self.title isEqual:(__bridge NSString *)kCUIAttrCredentialPassword];
}

@end
