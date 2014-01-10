//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class NSDictionary;

__attribute__((visibility("default")))
@interface CUICredential : NSObject

/*
 * Returns the set of CredUI attributes associated with this credential.
 * Attribute keys are specified in CredUICore/CUIAttributes.h.
 */
- (NSDictionary *)attributes;

/*
 * Acknowledges the application successfully used the credential and
 * it should be persisted. The application should only call this if
 * CUIFlagsExpectConfirmation was set, otherwise CredUI will do it.
 */
- (BOOL)savePersisted:(NSError * __autoreleasing *)error;

/*
 * If the credential was persisted, deletes it.
 */
- (BOOL)deletePersisted:(NSError * __autoreleasing *)error;

@end
