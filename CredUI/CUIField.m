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

CF_KVO_GETTERIMPLEMENTATION(CUIFieldClass,      fieldClass,     CUIFieldGetClass)
CF_KVO_GETTERIMPLEMENTATION(NSString *,         title,          CUIFieldGetTitle)
CF_KVO_GETTERIMPLEMENTATION(id,                 defaultValue,   CUIFieldGetDefaultValue)
CF_KVO_SETTERIMPLEMENTATION_COPY(id,            setValue,       CUIFieldSetValue)
CF_KVO_GETTERIMPLEMENTATION(CUIFieldOptions,    options,        CUIFieldGetOptions)
CF_KVO_SETTERIMPLEMENTATION(CUIFieldOptions,    setOptions,     CUIFieldSetOptions)

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

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)command
{
    if (command == @selector(insertNewline:)) {
        id identityPicker = [[(NSView *)textView window] delegate];
        
        [identityPicker willSubmitCredential:control];
        
        return YES;
    }
    
    return NO;
}

@end