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
    }
}

- (NSView *)_textFieldWithFrame:(NSRect)frame
{
    NSTextField *textField;
    
    if (self.defaultValue == nil)
        return nil;
    
    textField = [[NSTextField alloc] initWithFrame:frame];
    textField.stringValue = self.defaultValue;
    textField.editable = NO;
    textField.selectable = NO;
    [self _styleTextField:textField];
    
    textField.delegate = self;
    
    return textField;
}

- (NSView *)_editableTextFieldWithFrame:(NSRect)frame
{
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    if (self.defaultValue)
        textField.stringValue = self.defaultValue;
    textField.selectable = YES;
    textField.editable = YES;
    textField.delegate = self;
    [self _styleTextField:textField];

    return textField;
}

- (NSSecureTextField *)_secureTextFieldWithFrame:(NSRect)frame
{
    NSSecureTextField *textField = [[NSSecureTextField alloc] initWithFrame:frame];
    
    textField.selectable = NO;
    textField.editable = YES;
    textField.delegate = self;
    [self _styleTextField:textField];

    return textField;
}

- (NSButton *)_buttonWithFrame:(NSRect)frame
{
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = self.title;
    button.target = self;
    button.action = @selector(didSubmit:);
    
    return button;
}

- (NSView *)viewWithFrame:(NSRect)frame
{
    id view = nil;
    
    NSAssert(self.fieldClass != kCUIFieldClassInvalid, @"Field must be a valid type");
    NSAssert(self.fieldClass != kCUIFieldClassCustom,  @"Custom field classes must override viewWithFrame:");
    
    switch (self.fieldClass) {
        case kCUIFieldClassLargeText:
        case kCUIFieldClassSmallText:
            view = [self _textFieldWithFrame:frame];
            break;
        case kCUIFieldClassCommandLink:
            break;
        case kCUIFieldClassEditText:
            view = [self _editableTextFieldWithFrame:frame];
            break;
        case kCUIFieldClassPasswordText:
            view = [self _secureTextFieldWithFrame:frame];
            break;
        case kCUIFieldClassTileImage:
        case kCUIFieldClassCheckBox:
        case kCUIFieldClassComboBox:
            break;
        case kCUIFieldClassSubmitButton:
            // CUIIdentityPicker will draw a global submit button for all providers
            // view = [self _newButtonForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassInvalid:
        default:
            break;
    }
    
    return view;
}

@end
