//
//  GSSCredential+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation GSSCredential (CredUI)
- (instancetype)initWithCUICredential:(CUICredential *)cuiCredential error:(NSError **)error
{
    GSSCredential *cred = nil;
    GSSName *name = CFBridgingRelease([cuiCredential copyGSSName]);
    
    if (name) {
        cred = [self initWithName:name
                        mechanism:[GSSMechanism mechanismForCUICredential:cuiCredential]
                       attributes:[cuiCredential attributesWithClass:CUIAttributeClassGSSInitialCred]
                            error:error];
    }
    
    return cred;
}
@end