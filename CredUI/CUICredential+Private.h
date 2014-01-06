//
//  CUICredential+Private.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CoreFoundation/CFPlugInCOM.h>

#import <CredUICore/CUICredential.h>

@class CUIField;

@interface CUICredential () <NSSecureCoding>

@property(nonatomic, readonly, assign) NSArray *fields;

- (CUICredentialRef)_credentialRef;

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass;

- (void)didBecomeSelected:(BOOL *)pbAutoLogin;
- (void)didBecomeDeselected;

- (BOOL)canSubmit;
- (void)willSubmit;
- (void)didSubmit;



@end
