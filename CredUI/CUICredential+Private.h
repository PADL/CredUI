//
//  CUICredential+Private.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CoreFoundation/CFPlugInCOM.h>

#import <CredUICore/CUIField.h>
#import <CredUICore/CUICredential.h>

@class CUIField;

@interface CUICredential ()

@property(nonatomic, readonly, assign) NSArray *fields;

#ifdef __cplusplus
- initWithContext:(IUnknown *)context;
#else
- initWithContext:(struct IUnknownVTbl *)context;
#endif

- (CUICredentialRef)_credentialRef;

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass;

- (void)didBecomeSelected:(BOOL *)pbAutoLogin;
- (void)didBecomeDeselected;

- (void)willSubmit;
- (void)didSubmit;

#if 0
- (void)fieldsApplyBlock:(void (^)(CUIFieldRef, BOOL *))block
                    stop:(BOOL *)stop;
#endif

@end
