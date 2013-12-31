//
//  main.c
//  CredUIEnumerate
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <stdio.h>
#include <readpassphrase.h>

#include <Foundation/Foundation.h>

#include <CredUI/CredUI.h>
#include <CredUI/CUICredential.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CUIAttributes.h>

#include <GSSKit/GSSKit.h>

#include "../CredUICore/GSSItem.h"

static NSString *readFromConsole(NSString *prompt, NSString *defaultValue, BOOL echo)
{
    char buf[BUFSIZ], *s;
    NSString *p;
    
    if (defaultValue && echo)
        p = [NSString stringWithFormat:@"%@(%@): ", prompt, defaultValue];
    else
        p = [NSString stringWithFormat:@"%@: ", prompt];
    
    s = readpassphrase([p UTF8String], buf, sizeof(buf), echo ? RPP_ECHO_ON : RPP_ECHO_OFF);

    if (s == NULL)
        return nil;
    
    return [NSString stringWithUTF8String:s];
}

int main(int argc, const char * argv[])
{
    CUIControllerRef controller;
    NSDictionary *attributes = @{
                                 (__bridge id)kCUIAttrClass: (__bridge id)kCUIAttrClassKerberos,
                                 (__bridge id)kCUIAttrStatusTransient: @YES,
                                 (__bridge id)kCUIAttrStatusPersistant: @NO,
                                 (__bridge id)kCUIAttrNameType: (__bridge id)kCUIAttrNameTypeGSSUsername,
                                 (__bridge id)kCUIAttrName: @"lhoward@ATHENA.MIT.EDU"
                                };
    controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, kCUIUsageFlagsNoUI);
    if (controller == NULL) {
        NSLog(@"failed to create controller");
        exit(1);
    }
    
    CUIControllerSetAttributes(controller, (__bridge CFDictionaryRef)attributes);
    
    NSLog(@"Controller is: %@", (__bridge id)controller);
    NSLog(@"Attributes are: %@", attributes);
    
    NSMutableArray *creds = [NSMutableArray array];
    
    CUIControllerEnumerateCredentials(controller, ^(CUICredentialRef cred, CFErrorRef err) {
        if (cred)
            [creds addObject:(__bridge id)cred];
    });

    [creds enumerateObjectsUsingBlock:^(id cred, NSUInteger idx, BOOL *stop) {
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass((__bridge CUICredentialRef)cred, kCUIFieldClassLargeText);
        CUIFieldRef field2 = CUICredentialFindFirstFieldWithClass((__bridge CUICredentialRef)cred, kCUIFieldClassEditText);
        
        if (field) {
            NSLog(@"[%lu] %@(%@)", (unsigned long)idx, (__bridge NSString *)CUIFieldGetTitle(field), (__bridge NSString *)CUIFieldGetDefaultValue(field2));
        }
    }];
    
    CUICredentialRef cred = NULL;
    
    if ([creds count] > 1) {
        NSString *which = readFromConsole(@"Select credential", nil, true);
        cred = (__bridge CUICredentialRef)[creds objectAtIndex:[which integerValue]];
    } else if ([creds count] == 1) {
        cred = (__bridge CUICredentialRef)[creds objectAtIndex:0];
    } else {
        NSLog(@"No credentials");
        exit(1);
    }
    
    Boolean autoLogin = false;
    CUICredentialDidBecomeSelected(cred, &autoLogin);
    
    CUICredentialFieldsApplyBlock(cred, ^(CUIFieldRef field, Boolean *stop) {
        NSString *title = (__bridge NSString *)CUIFieldGetTitle(field);
        NSString *value = nil;
        NSString *defaultValue = (__bridge NSString *)CUIFieldGetDefaultValue(field);

        switch (CUIFieldGetClass(field)) {
            case kCUIFieldClassLargeText:
                break;
            case kCUIFieldClassSmallText:
                printf("%s\n", [title UTF8String]);
                break;
            case kCUIFieldClassEditText:
                value = readFromConsole(title, defaultValue, true);
                break;
            case kCUIFieldClassPasswordText:
                value = readFromConsole(title, defaultValue, false);
                break;
            default:
                NSLog(@"Ignoring field %@", (__bridge id)field);
                break;
        }
        
        if (value && [value length])
            CUIFieldSetValue(field, (__bridge CFTypeRef)value);
    });

    NSDictionary *credAttributes = [(__bridge CUICredential *)cred attributesWithClass:CUIAttributeClassGSSItem];
    
    NSLog(@"Credential attributes: %@", credAttributes);
    
    NSError *error = NULL;
    GSSItem *item = (__bridge GSSItem *)CUICredentialGetGSSItem(cred);
    
    if (item) {
        GSSCredential *cred = [item acquire:credAttributes error:&error];
        NSLog(@"Got cred: %@", cred);
    } else if (error) {
        NSLog(@"Failed to add item for attributes: %@", error);
    } else {
        NSLog(@"Failed to add item");
    }
    
    CFRelease(controller);
    
    exit(0);
}
