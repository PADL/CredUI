//
//  GSSMechanism+CredUI.h
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <GSSKit/GSSKit.h>

@class CUICredential;

/*
 * This can go in CredUI.framework eventually, but I want to keep the dependencies
 * to a minimum for now.
 */

@interface GSSMechanism (CredUI)

+ (GSSMechanism *)mechanismForCUICredential:(CUICredential *)cuiCredential;

@end
