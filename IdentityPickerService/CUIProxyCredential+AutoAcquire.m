//
//  CUICredential+AutoAcquire.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <GSSKit/GSSKit.h>
#import <GSSKitUI/GSSKitUI.h>

#import <CredUICore/CUIAttributes.h>

#import "CUIProxyCredential+AutoAcquire.h"

@implementation CUIProxyCredential (AutoAcquire)

- (BOOL)autoAcquireGSSCred:(NSError * __autoreleasing *)error
{
    gss_cred_id_t gssCred = [self acquireGSSCredential:error];

    if (gssCred == GSS_C_NO_CREDENTIAL)
        return NO;
    
    [(NSMutableDictionary *)self.attributes setObject:CFBridgingRelease(gssCred) forKey:(__bridge id)kCUIAttrGSSCredential];
    return YES;
}

@end
