//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class CUIField;

@interface CUICredential : NSObject

@property(nonatomic, readonly, assign) NSArray *fields;

- initWithContext:(IUnknownVTbl *)context;

- (CUICredentialRef)_credentialRef;

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass;

- (NSDictionary *)attributes;

- (void)didBecomeSelected:(BOOL *)pbAutoLogin;
- (void)didBecomeDeselected;

- (void)fieldsApplyBlock:(void (^)(CUIFieldRef, BOOL *))block
                    stop:(BOOL *)stop;

- (id)_createGSSItem:(BOOL)addIfNotExisting error:(NSError * __autoreleasing *)error;

@end
