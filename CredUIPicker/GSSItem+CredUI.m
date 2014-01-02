//
//  GSSItem+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>

#import "GSSItem+CredUI.h"

@implementation GSSItem (CredUI)
+ (NSArray *)itemsMatchingCUICredential:(CUICredential *)cuiCredential error:(NSError **)error
{
    return [GSSItem copyMatching:[cuiCredential attributesWithClass:CUIAttributeClassGSSItem] error:error];
}
@end
