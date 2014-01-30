//
//  CUICredential+ViewBridge.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <GSSKitUI/GSSKitUI.h>
#import <CredUICore/CredUICore.h>

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

@end
