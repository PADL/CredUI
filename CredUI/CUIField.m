//
//  CUIField.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#pragma mark - CUICFField concrete class

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-implementations"
CF_CLASSIMPLEMENTATION(CUICFField)
#pragma clang diagnostic pop

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey
{
    return NO;
}

- init
{
    [super init];
    NSAssert(0, @"CUICFField can only be instantiated through CredUICore");
    return nil;
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

- (CUIFieldOptions)options
{
    return CUIFieldGetOptions([self _fieldRef]);
}

- (void)setOptions:(CUIFieldOptions)options
{
    CUIFieldSetOptions([self _fieldRef], options);
}

- (void)setValue:(id)aValue
{
    id aValueCopy = [aValue copy];
    
    CUIFieldSetValue([self _fieldRef], (CFTypeRef)aValueCopy);
    
    [aValueCopy release];
}

@end

#pragma mark - CUIField abstract class

@implementation CUIField

- (CUIFieldRef)_fieldRef
{
    return (CUIFieldRef)self;
}

- (CFTypeID)_cfTypeID
{
    return CUIFieldGetTypeID();
}

#pragma mark Default methods

- (id)valueForUndefinedKey:(NSString *)key
{
    /* The value is actually write-only, so just return a defaultValue for KVO compliance */
    if ([key isEqualToString:@"value"])
        return [self defaultValue];
    else
        return [super valueForUndefinedKey:key];
}

- (void)didSubmit:(id)sender
{
    [self setValue:@YES];
}

- (void)controlTextDidChange:(NSNotification *)notification
{
    NSTextView *textView = [notification.userInfo objectForKey:@"NSFieldEditor"];
    
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

#pragma mark Primitive methods

- (void)setValue:(id)aValue
{
    NSRequestConcreteImplementation(self, _cmd, [CUIField class]);
}

- (void)setOptions:(CUIFieldOptions)options
{
    NSRequestConcreteImplementation(self, _cmd, [CUIField class]);
}

- (CUIFieldClass)fieldClass
{
    return kCUIFieldClassInvalid;
}

- (NSString *)title
{
    return nil;
}

- (id)defaultValue
{
    return nil;
}

- (CUIFieldOptions)options
{
    return kCUIFieldOptionsNone;
}

@end
