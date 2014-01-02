//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class NSDictionary;
@class GSSItem;

__attribute__((visibility("default")))
@interface CUICredential : NSObject

/*
 * Returns the set of CredUI attributes associated with this credential.
 * Attribute keys are specified in CredUICore/CUIAttributes.h.
 */
- (NSDictionary *)attributes;

/*
 * Returns a mapped set of attributes for a particular usage, such as
 * adding/updating a GSSItem or acquiring initial GSS-API credentials.
 */
- (NSDictionary *)attributesWithClass:(CUIAttributeClass)flags;

/*
 * Acknowledges the application successfully used the credential and
 * it should be persisted.
 */
- (BOOL)didConfirm:(NSError * __autoreleasing *)error;

/*
 * A GSS item object; this can be cast to a GSSItemRef. It is autoreleased.
 */
- (GSSItem *)GSSItem;

/*
 * A GSS name object for the credential initiator; this can be cast to
 * a gss_name_t or a GSSName * if using GSSKit. It is autoreleased.
 */
- (id)GSSName;

@end