//
//  GSSCredential+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>

#import "GSSCredential+CredUI.h"

@implementation GSSCredential (CredUI)
- (instancetype)initWithCUICredential:(CUICredential *)cuiCredential error:(NSError **)error
{
    GSSCredential *cred = nil;
    GSSName *name = cuiCredential.GSSName;
    
    if (name) {
        cred = [self initWithName:name
                        mechanism:[GSSMechanism mechanismWithClass:cuiCredential.attributes[@"kCUIAttrClass"]]
                       attributes:[cuiCredential attributesWithClass:CUIAttributeClassGSSInitialCred]
                            error:error];
    }
    
    return cred;
}
@end
