//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTile

- (NSTextField *)_newTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSTextField *textField;
    
    if (field.defaultValue == nil)
        return nil;
    
    textField = [[NSTextField alloc] initWithFrame:frame];
    textField.stringValue = field.defaultValue;
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

- (NSTextField *)_newEditableTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    if (field.defaultValue)
        textField.stringValue = field.defaultValue;
    textField.selectable = YES;
    textField.editable = YES;
    textField.delegate = field;
    
    return textField;
}

- (NSSecureTextField *)_newSecureTextFieldForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSSecureTextField *textField = [[NSSecureTextField alloc] initWithFrame:frame];
    
    textField.selectable = NO;
    textField.editable = YES;
    textField.delegate = field;

    return textField;
}

- (NSButton *)_newButtonForCredentialField:(CUIField *)field withFrame:(NSRect)frame
{
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = field.title;
    button.target = field;
    button.action = @selector(didSubmit:);

    return button;
}
    
- (NSView *)_newViewForCredentialField:(CUIField *)field withFrame:(NSRect *)frame
{
    NSView *view = nil;
    
    frame->size.height = 25;
    frame->origin.y -= frame->size.height;

    switch ([field fieldClass]) {
        case kCUIFieldClassLargeText:
        case kCUIFieldClassSmallText:
            view = [self _newTextFieldForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassCommandLink:
            break;
        case kCUIFieldClassEditText:
            view = [self _newEditableTextFieldForCredentialField:field withFrame:*frame];
            break;
        case kCUIFieldClassPasswordText:
            view = [self _newSecureTextFieldForCredentialField:field withFrame:*frame];
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

    if (view)
        [field addObserver:self.tileController forKeyPath:@"hidden" options:NSKeyValueObservingOptionNew context:(__bridge void *)view];

    return view;
}

- (void)dealloc
{
    NSArray *credFields = [self.credential fields];

    for (CUIField *field in credFields) {
        [field removeObserver:self.tileController forKeyPath:@"hidden"];
    }
}

- (void)_updateSubviews
{
    NSArray *credFields = [self.credential fields];
    NSRect frame = self.frame;
    NSView *lastview = nil;
    
    frame.origin.y = frame.size.height;
    frame.size.height = 0;
    
    for (CUIField *field in credFields) {
        NSView *subview = [self _newViewForCredentialField:field withFrame:&frame];
        
        if (subview) {
            if (lastview)
                lastview.nextKeyView = subview;
            [self addSubview:subview];
            lastview = subview;
        }
    }
}

- (void)setCredential:(CUICredential *)credential
{
    _credential = credential;
    [self _updateSubviews];
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (self.selected) {
        [[NSColor alternateSelectedControlColor] set];
        NSRectFill([self bounds]);
    }
}

@end
