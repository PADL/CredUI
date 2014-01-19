//
//  CUICredential+GSSKitUI.h
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <GSS/GSS.h>

struct GSSItem;

typedef NS_ENUM(NSUInteger, CUIAttributeClass) {
    CUIAttributeClassGeneric                = 1,    // generic credential attributes
    CUIAttributeClassGSSInitialCred         = 2,    // attributes for gss_aapl_initial_cred
    CUIAttributeClassGSSItem                = 4     // attributes for GSSItemAdd
};

@interface CUICredential (GSSKitUI)

/*
 * Returns a mapped set of attributes for a particular usage, such as
 * adding/updating a GSSItem or acquiring initial GSS-API credentials.
 */
- (NSDictionary *)attributesWithClass:(CUIAttributeClass)flags;

/*
 * A GSS name object for the credential initiator.
 */
- (gss_name_t)copyGSSName CF_RETURNS_RETAINED;

/*
 * GSS item for the credential.
 */
- (struct GSSItem *)copyMatchingGSSItem:(BOOL)addIfAbsent error:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED;

/*
 * Acquire GSS credential.
 */
- (gss_cred_id_t)acquireGSSCredential:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED;

@end
