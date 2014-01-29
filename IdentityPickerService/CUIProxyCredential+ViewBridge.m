//
//  CUICredential+ViewBridge.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <GSSKit/GSSKit.h>
#import <GSSKitUI/GSSKitUI.h>

#import <CredUICore/CUIAttributes.h>
#import <CredUI/CUICredential+CBIdentity.h>

#import "CUIProxyCredential+ViewBridge.h"

@implementation CUIProxyCredential (ViewBridge)

- (BOOL)acquireAndSetGSSCredential
{
    NSError *error = nil;
    gss_cred_id_t gssCred = [self acquireGSSCredential:&error];

    if (gssCred)
        [(NSMutableDictionary *)self.attributes setObject:CFBridgingRelease(gssCred) forKey:(__bridge id)kCUIAttrGSSCredential];
    if (error)
        [(NSMutableDictionary *)self.attributes setObject:error forKey:(__bridge id)kCUIAttrCredentialError];

    return !!gssCred;
}

- (BOOL)authAndSetAuthenticatedForLoginScenario
{
    BOOL bAuthenticated = [self authenticateForLoginScenario];

    [(NSMutableDictionary *)self.attributes setObject:[NSNumber numberWithBool:bAuthenticated] forKey:(__bridge id)kCUIAttrAuthenticatedForLoginScenario];

    return bAuthenticated;
}

@end
