//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTile

- (NSTextField *)_createTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    textField.stringValue = field.title;
    textField.editable = NO;
    textField.selectable = NO;
    if ([field fieldClass] == kCUIFieldClassLargeText) {
        textField.bordered = YES;
        textField.bezeled = YES;
        textField.backgroundColor = [NSColor darkGrayColor];
    }
    textField.delegate = field;
    
    return textField;
}

- (NSTextField *)_createEditableTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    if (field.defaultValue)
        textField.stringValue = field.defaultValue;
    textField.selectable = YES;
    textField.editable = YES;
    textField.delegate = field;
    return textField;
}

- (NSSecureTextField *)_createSecureTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSSecureTextField *textField = [[NSSecureTextField alloc] initWithFrame:frame];
    
    textField.selectable = NO;
    textField.editable = YES;
    textField.delegate = field;

    return textField;
}

- (NSButton *)_createButtonForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = field.title;
    button.target = field;
    button.action = @selector(didSubmit:);

    return button;
}
    
- (NSView *)_createViewForCredentialField:(CUIField *)field withFrame:(NSRect *)frame
{
    NSView *view = nil;
    
    frame->size.height = 25;
    frame->origin.y -= frame->size.height;

    switch ([field fieldClass]) {
        case kCUIFieldClassLargeText:
        case kCUIFieldClassSmallText:
            view = [self _createTextFieldForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassCommandLink:
            break;
        case kCUIFieldClassEditText:
            view = [self _createEditableTextFieldForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassPasswordText:
            view = [self _createSecureTextFieldForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassTileImage:
        case kCUIFieldClassCheckBox:
        case kCUIFieldClassComboBox:
            break;
        case kCUIFieldClassSubmitButton:
            view = [self _createButtonForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassInvalid:
        default:
            break;
    }
 
    return view;
}

- (void)_updateSubviews
{
    NSArray *credFields = [self.credential fields];
    NSRect frame = self.frame;
    
    frame.origin.y = frame.size.height;
    frame.size.height = 0;
    
    for (CUIField *field in credFields) {
        NSView *subview = [self _createViewForCredentialField:field withFrame:&frame];
        
        if (subview) {
            [self addSubview:subview];
        }
    }
}

- (void)setCredential:(CUICredential *)credential
{
    _credential = credential;
    [self _updateSubviews];
}

@end
