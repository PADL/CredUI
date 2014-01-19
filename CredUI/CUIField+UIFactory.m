//
//  CUIField+UIFactory.m
//  CredUI
//
//  Created by Luke Howard on 7/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIField (UIFactory)

- (void)_styleTextField:(NSTextField *)textField
{
    [textField setBackgroundColor:[NSColor clearColor]];
   
    if (textField.isEditable) { 
        if (self.title)
            [textField.cell setPlaceholderString:self.title];

        textField.bezeled = YES;
    } else {
        textField.bezeled = NO;
    }
}

- (NSView *)_newTextField
{
    NSTextField *textField;
    
    if (self.defaultValue == nil)
        return nil;
    
    textField = [[NSTextField alloc] init];
    textField.stringValue = self.defaultValue;
    textField.editable = NO;
    textField.selectable = NO;
    [self _styleTextField:textField];
    
    textField.delegate = self;
    
    return textField;
}

- (NSView *)_newEditableTextField
{
    NSTextField *textField = [[NSTextField alloc] init];
    
    if (self.defaultValue)
        textField.stringValue = self.defaultValue;
    textField.selectable = YES;
    textField.editable = YES;
    textField.delegate = self;
    [self _styleTextField:textField];

    return textField;
}

- (NSSecureTextField *)_newSecureTextField
{
    NSSecureTextField *textField = [[NSSecureTextField alloc] init];
    
    textField.selectable = NO;
    textField.editable = YES;
    textField.delegate = self;
    [self _styleTextField:textField];

    return textField;
}

- (NSButton *)_newButton
{
    NSButton *button = [[NSButton alloc] init];
    
    button.title = self.title;
    button.target = self;
    button.action = @selector(didSubmit:);
    
    return button;
}

- (NSView *)view
{
    NSView *view = nil;

    NSAssert(self.fieldClass != kCUIFieldClassInvalid, @"Field must be a valid type");
    NSAssert(self.fieldClass != kCUIFieldClassCustom,  @"Custom field classes must override view");
    
    switch (self.fieldClass) {
        case kCUIFieldClassLargeText:
        case kCUIFieldClassSmallText:
            view = [self _newTextField];
            break;
        case kCUIFieldClassCommandLink:
            break;
        case kCUIFieldClassEditText:
            view = [self _newEditableTextField];
            break;
        case kCUIFieldClassPasswordText:
            view = [self _newSecureTextField];
            break;
        case kCUIFieldClassTileImage:
        case kCUIFieldClassCheckBox:
        case kCUIFieldClassComboBox:
            break;
        case kCUIFieldClassSubmitButton:
            // CUIIdentityPicker will draw a global submit button for all providers
            break;
        case kCUIFieldClassInvalid:
        default:
            break;
    }
    
    return view;
}

@end
