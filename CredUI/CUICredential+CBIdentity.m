//
//  CUICredential+CBIdentity.m
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Collaboration/Collaboration.h>

@implementation CUICredential (CBIdentity)
- (BOOL)authenticateForLoginScenario:(NSString *)service
{
    return CUICredentialAuthenticateForLoginScenario([self _credentialRef], (__bridge CFStringRef)service);
}

- (CBUserIdentity *)userIdentity
{
    CBIdentity *identity;
    CSIdentityRef csIdentity = (__bridge CSIdentityRef)self.attributes[(__bridge id)kCUIAttrCSIdentity];
    
    if (csIdentity != nil) {
        identity = [CBIdentity identityWithCSIdentity:csIdentity];
    } else {
        NSString *name = self.attributes[(__bridge id)kCUIAttrName];

        if (name)
            identity = [CBIdentity identityWithName:name authority:[CBIdentityAuthority defaultIdentityAuthority]];
    }

    if ([identity isKindOfClass:[CBUserIdentity class]])
        return (CBUserIdentity *)identity;
    
    return nil;
}
@end
