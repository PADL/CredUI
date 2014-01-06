//
//  CustomField.cpp
//  CredUI
//
//  Created by Luke Howard on 7/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CredUI/CredUI.h>
#include <CredUI/CUIField.h>

#include "CustomField.h"

@interface CustomField : CUIField
@property (nonatomic) SampleCredential *credential;
@end

@implementation CustomField

- (void)setValue:(id)aValue
{
    NSLog(@"CustomField: setting username to %@", aValue);

    self.credential->setUsername((__bridge CFStringRef)aValue);
}

- (NSView *)viewWithFrame:(NSRect)frame
{
    NSTextField *textField;
    NSString *defaultValue = (__bridge NSString *)self.credential->getDefaultUsername();
    
    textField = [[NSTextField alloc] initWithFrame:frame];
    if (defaultValue)
        textField.stringValue = defaultValue;
    textField.editable = YES;
    textField.selectable = YES;
    textField.bordered = YES;
    textField.bezeled = YES;
    textField.backgroundColor = [NSColor darkGrayColor];
    textField.delegate = self;
    
    return textField;
}

- (CUIFieldClass)fieldClass
{
    return kCUIFieldClassEditText;
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
}

@end

CUIFieldRef
CustomFieldCreate(SampleCredential *sampleCred)
{
    CustomField *customField;
    
    customField = [[CustomField alloc] init];
    customField.credential = sampleCred;
    
    return (CUIFieldRef)CFBridgingRetain(customField);
}
