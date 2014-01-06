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
@property (nonatomic, retain) NSMutableDictionary *attributes;
@end

@implementation CustomField

- (NSString *)defaultUsername
{
    NSString *defaultUserName;
    
    defaultUserName = self.attributes[(__bridge id)kCUIAttrNameDisplay];
    if (!defaultUserName) {
        id nameType = self.attributes[(__bridge id)kCUIAttrNameType];
        if (nameType && [nameType isEqual:(__bridge id)kCUIAttrNameTypeGSSUsername])
            defaultUserName = self.attributes[(__bridge id)kCUIAttrName];
    }
    
    return defaultUserName;
}

- (void)setValue:(id)aValue
{
    NSLog(@"CustomField: setting username to %@", aValue);

    self.attributes[(__bridge id)kCUIAttrNameType] = (__bridge id)kCUIAttrNameTypeGSSUsername;
    self.attributes[(__bridge id)kCUIAttrName] = [aValue copy];
}

- (NSView *)viewWithFrame:(NSRect)frame
{
    NSTextField *textField;
    NSString *defaultValue = [self defaultUsername];
    
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

@end

CUIFieldRef
CustomFieldCreate(CFMutableDictionaryRef attributes)
{
    CustomField *customField;
    
    customField = [[CustomField alloc] init];
    customField.attributes = (__bridge NSMutableDictionary *)attributes;
    
    return (CUIFieldRef)CFBridgingRetain(customField);
}
