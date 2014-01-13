//
//  CUICredential+CBIdentity.h
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Collaboration/CBIdentity.h>
#import <CredUI/CUICredential.h>

@interface CUICredential (CBIdentity)
- (CBUserIdentity *)userIdentity;
- (BOOL)authenticateForLoginScenario:(NSString *)service;
@end
