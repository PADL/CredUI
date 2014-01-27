//
//  CustomUsernameField.cpp
//  CredUI
//
//  Created by Luke Howard on 7/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CredUI/CredUI.h>
#include <CredUI/CUIField.h>

#include "CustomUsernameField.h"

/*
 * Providers generally should only create fields using CUIFieldCreate() as it avoids
 * creating a Cocoa dependency and ensures providers can be used with non-UI applications.
 *
 * However as an example we demonstrate subclassing CUIField to display a custom field.
 * Subclasses must implement the setValue: and viewWithFrame: primitive methods.
 */

@interface CustomUsernameField : CUIField
{
    SampleCredential *_credential;
}
@property (nonatomic) SampleCredential *credential;
@end

@implementation CustomUsernameField
@synthesize credential = _credential;

- (NSView *)view
{
    NSTextField *textField;
    
    textField = [[NSTextField alloc] init];
    if (self.defaultValue)
        textField.stringValue = self.defaultValue;
    else if (self.title)
        [textField.cell setPlaceholderString:self.title];

    textField.editable = YES;
    textField.selectable = YES;
    textField.bezeled = YES;
    textField.backgroundColor = [NSColor greenColor];
    textField.delegate = self;
    
    return textField;
}

- (CUIFieldClass)fieldClass
{
    return kCUIFieldClassEditText;
}

- (NSString *)title
{
    return @"Username";
}

- (NSString *)defaultValue
{
    return (__bridge NSString *)self.credential->getDefaultUsername();
}

- (void)setValue:(id)aValue
{
    NSLog(@"CustomUsernameField: setting username to %@", aValue);

    self.credential->setUsername((__bridge CFStringRef)[aValue copy]);
    self.credential->updateCredentialStatus();
}

- (void)setCredential:(SampleCredential *)credential
{
    if (_credential && _credential != credential)
        _credential->Release();
    credential->AddRef();
    _credential = credential;
}

- (void)dealloc
{
    if (_credential)
        _credential->Release();
    
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

@end

CUIFieldRef
CustomUsernameFieldCreate(SampleCredential *sampleCred)
{
    CustomUsernameField *customUsernameField;
    
    customUsernameField = [[CustomUsernameField alloc] init];
    customUsernameField.credential = sampleCred;
    
    return (CUIFieldRef)CFBridgingRetain(customUsernameField);
}
