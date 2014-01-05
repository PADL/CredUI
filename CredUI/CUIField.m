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
    static CUIFieldRef placeholderField;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (placeholderField == NULL)
            placeholderField = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassInvalid,
                                              NULL, NULL, NULL);
    });
    
    return (id)placeholderField;
}

CF_CLASSIMPLEMENTATION(CUICFField)

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey
{
    return NO;
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

- (id)valueForUndefinedKey:(NSString *)key
{
    /* The value is actually write-only, so just return a defaultValue for KVO compliance */
    if ([key isEqualToString:@"value"])
        return [self defaultValue];
    else
        return [super valueForUndefinedKey:key];
}

- (void)setValue:(id)aValue
{
    id aValueCopy = [aValue copy];

    CUIFieldSetValue([self _fieldRef], (CFTypeRef)aValueCopy);

    [aValueCopy release];
}

- (CUIFieldOptions)options
{
    return CUIFieldGetOptions([self _fieldRef]);
}

- (void)setOptions:(CUIFieldOptions)options
{
    CUIFieldSetOptions([self _fieldRef], options);
}

- (void)didSubmit:(id)sender
{
    [self setValue:(id)kCFBooleanTrue];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextView *textView = notification.userInfo[@"NSFieldEditor"];
    
    [self setValue:textView.string];
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
