//
//  CUICredential+CBIdentity.m
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Collaboration/Collaboration.h>

@implementation CUICredential (CBIdentity)
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

- (CBUserIdentity *)authenticatedUserIdentity
{
    CBUserIdentity *userIdentity = [self userIdentity];
    NSString *password = self.attributes[(__bridge id)kCUIAttrCredentialPassword];

    return (userIdentity && password && [userIdentity authenticateWithPassword:password]) ? userIdentity : nil;
}
@end
