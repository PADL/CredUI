//
//  GSSMechanism+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>

#import "GSSMechanism+CredUI.h"

@implementation GSSMechanism (CredUI)
+ (GSSMechanism *)mechanismForCUICredential:(CUICredential *)cuiCredential
{
    return [self mechanismWithClass:cuiCredential.attributes[@"kCUIAttrClass"]];
}
@end
