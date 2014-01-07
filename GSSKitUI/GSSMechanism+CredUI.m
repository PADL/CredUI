//
//  GSSMechanism+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation GSSMechanism (CredUI)
+ (GSSMechanism *)mechanismForCUICredential:(CUICredential *)cuiCredential
{
    GSSMechanism *mech = [self mechanismWithClass:cuiCredential.attributes[(__bridge id)kCUIAttrClass]];
    
    if ([mech isKerberosMechanism] && cuiCredential.attributes[(__bridge id)kCUIAttrCredentialSecIdentity])
        mech = [GSSMechanism IAKerbMechanism];
    
    return mech;
}
@end
