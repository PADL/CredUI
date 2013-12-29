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
#include <CredUICore/CredUICore.h>

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
    NSDictionary *authIdentity = @{
                                   };
    controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, kCUIUsageFlagsNoUI);
    if (controller == NULL) {
        NSLog(@"failed to create controller");
        exit(1);
    }
    
    CUIControllerSetAuthIdentity(controller, (__bridge CFDictionaryRef)authIdentity);
    
    NSLog(@"Controller is: %@", (__bridge id)controller);
    NSLog(@"Auth identity is: %@", authIdentity);
    
    NSMutableArray *creds = [NSMutableArray array];
    
    CUIControllerEnumerateCredentials(controller, ^(CUICredentialRef cred) {
        [creds addObject:(__bridge id)cred];
    });

    [creds enumerateObjectsUsingBlock:^(id cred, NSUInteger idx, BOOL *stop) {
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass((__bridge CUICredentialRef)cred, kCUIFieldClassLargeText);
    
        if (field) {
            NSLog(@"[%lu] %@", (unsigned long)idx, (__bridge NSString *)CUIFieldGetTitle(field));
        }
    }];
    
    NSString *which = readFromConsole(@"Select credential", nil, true);
    
    CUICredentialRef cred = (__bridge CUICredentialRef)[creds objectAtIndex:[which integerValue]];
    Boolean stop = false;
    
    CUICredentialDidBecomeSelected(cred);
    
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
        
        if (value)
            CUIFieldSetValue(field, (__bridge CFTypeRef)value);
    }, &stop);

    NSDictionary *credAuthIdentity = (__bridge NSDictionary *)CUICredentialGetAuthIdentity(cred);
    
    NSLog(@"Auth identity: %@", credAuthIdentity);
    
    exit(0);
}