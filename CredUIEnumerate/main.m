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
#include <Cocoa/Cocoa.h>

#include <CredUI/CredUI.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/CUIAttributes.h>

#include <CredUI/CUICredential.h>
#include <CredUI/CUIField.h>

#include <GSSKit/GSSKit.h>
#include <GSSKitUI/GSSKitUI.h>

#include "GSSItem.h"

@interface FooCredential : CUICredential
@end

@class CUIField;

@implementation FooCredential
- init
{
    self = [super init];
    return self;
}

+ allocWithZone:(NSZone *)zone
{
    id a = [super allocWithZone:zone];
    
    return a;
}

- (void)didBecomeSelected:(BOOL *)foo
{
    NSLog(@"didBecomeSelected");
    *foo = YES;
}

- (NSString *)description
{
    return @"Hello, this is a custom field";
}

- (NSArray *)fields
{
    NSLog(@"FooCredential: fields");
    return nil;
}

@end

static void testSubclasses(void)
{
    FooCredential *foo = [[FooCredential alloc] init];
    CUICredentialGetFields((__bridge CUICredentialRef)foo);
    
    NSLog(@"cred: %@", foo);
    
    CFStringRef cfDesc = CFCopyDescription((__bridge CFTypeRef)foo);
    if (cfDesc) {
        NSLog(@"CF desc: %@", cfDesc);
        CFRelease(cfDesc);
    }
    
    Boolean bar;
    
    CUICredentialDidBecomeSelected((__bridge CUICredentialRef)foo, &bar);
    NSLog(@"Did become selected = %d", bar);
}

static void testEncodeDecode(CUICredentialRef cred)
{
    NSString *filePath = @"/tmp/somecred";
    
    [NSKeyedArchiver archiveRootObject:(__bridge id)cred toFile:filePath];
    
    id reconstitutedCred = [NSKeyedUnarchiver unarchiveObjectWithData:[NSData dataWithContentsOfFile:filePath]];
    
    NSLog(@"reconstitutedCred: %@", reconstitutedCred);
}

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
                                 //(__bridge id)kCUIAttrName: @"lukeh@DE.PADL.COM"
                                };
    
    testSubclasses();
    
    controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioNetwork, kCUIUsageFlagsConsole);
    if (controller == NULL) {
        NSLog(@"failed to create controller");
        exit(1);
    }
    
    CUIControllerSetAttributes(controller, (__bridge CFDictionaryRef)attributes);
    
    NSLog(@"Controller is: %@", (__bridge id)controller);
    NSLog(@"IN_CRED attributes are: %@", attributes);
    
    NSMutableArray *creds = [NSMutableArray array];
    
    CUIControllerEnumerateCredentials(controller, ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
        if (cred)
            [creds addObject:(__bridge id)cred];
    });

    [creds enumerateObjectsUsingBlock:^(id cred, NSUInteger idx, BOOL *stop) {
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass((__bridge CUICredentialRef)cred, kCUIFieldClassLargeText);
        CUIFieldRef field2 = CUICredentialFindFirstFieldWithClass((__bridge CUICredentialRef)cred, kCUIFieldClassEditText);
        
        if (field) {
            NSLog(@"[%lu] %@(%@)", (unsigned long)idx,
                  field ? (__bridge NSString *)CUIFieldGetTitle(field) : nil,
                  field2 ? (__bridge NSString *)CUIFieldGetDefaultValue(field2) : nil);
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
    
    NSLog(@"OUT_CRED attributes: %@", credAttributes);
    
    NSError *error = nil;
    GSSItem *item = CFBridgingRelease([(__bridge CUICredential *)cred copyMatchingGSSItem:YES error:&error]);
    if (item) {
        GSSCredential *cred;
        
        cred = [item acquire:credAttributes error:&error];
        
        NSLog(@"Got cred: %@", cred);
    } else if (error) {
        NSLog(@"Failed to find item for attributes: %@", error);
    } else {
        NSLog(@"Failed to find item");
    }
    
    testEncodeDecode(cred);
    
    CFRelease(controller);
    
    exit(0);
}
