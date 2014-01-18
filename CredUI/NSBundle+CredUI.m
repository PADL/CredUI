//
//  NSBundle+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 17/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation NSBundle (CredUI)

+ (NSBundle *)credUIBundle
{
    static dispatch_once_t onceToken;
    static NSBundle *credUIBundle = nil;
    
    dispatch_once(&onceToken, ^{
        credUIBundle = [NSBundle bundleForClass:[CUIIdentityPickerInternal class]];
    });
    
    return credUIBundle;
}

@end
