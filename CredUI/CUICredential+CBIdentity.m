//
//  CUICredential+CBIdentity.m
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Collaboration/Collaboration.h>

@implementation CUICFCredential (CBIdentity)
- (BOOL)authenticateForLoginScenario
{
    return CUICredentialAuthenticateForLoginScenario([self _credentialRef]);
}
@end

@implementation CUICredential (CBIdentity)

- (CBUserIdentity *)userIdentity
{
    CBIdentity *identity = nil;
    CSIdentityRef csIdentity = (__bridge CSIdentityRef)[self.attributes objectForKey:(__bridge id)kCUIAttrCSIdentity];
    
    if (csIdentity != nil) {
        identity = [CBIdentity identityWithCSIdentity:csIdentity];
    } else {
        NSString *name = [self.attributes objectForKey:(__bridge id)kCUIAttrName];

        if (name)
            identity = [CBIdentity identityWithName:name authority:[CBIdentityAuthority defaultIdentityAuthority]];
    }

    if ([identity isKindOfClass:[CBUserIdentity class]])
        return (CBUserIdentity *)identity;
    
    return nil;
}

- (BOOL)authenticateForLoginScenario
{
    return [[self.attributes valueForKey:(__bridge id)kCUIAttrAuthenticatedForLoginScenario] boolValue];
}

@end
