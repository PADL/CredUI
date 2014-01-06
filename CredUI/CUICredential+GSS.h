//
//  CUICredential+GSS.h
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <GSS/GSS.h>
#import <CredUI/CUICredential.h>

@interface CUICredential (GSS)
/*
 * A GSS name object for the credential initiator.
 */
- (gss_name_t)copyGSSName CF_RETURNS_RETAINED;

/*
 * GSS item for the credential.
 */
struct GSSItem;

- (struct GSSItem *)copyMatchingGSSItem:(BOOL)addIfAbsent error:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED;

/*
 * Why is there no method to acquire a credential? gss_aapl_initial_cred() does not
 * yet have enough logic to support arbitrary credential dictionaries, and I don't
 * want do duplicate the workaround logic (which currently is implemented in GSSKit)
 * or introduce a dependency on GSSKit at this stage.
 *
 * You *could* acquire an initial credential using the following API:
 *
 *   OM_uint32 major, minor;
 *   CUICredential *cuiCred = ...;
 *   gss_name_t name = [cuiCred copyGSSName];
 *   CFDictionaryRef attrs = (__bridge CFDictionaryRef)[cuiCred attributesWithClass:CUIAttributeClassGSSInitialCred];
 *   gss_cred_id_t cred = GSS_C_NO_CREDENTIAL;
 *   CFErrorRef err;
 *
 *   major = gss_aapl_initial_cred(name, GSS_SPNEGO_MECHANISM, attrs, &cred, NULL);
 *
 * However, It is recommended instead that you use GSSKitUI, which let's you do the
 * following:
 *
 *   GSSCredential *cred = [GSSCredential intiWithCUICredential:cuiCred error:&error];
 *
 * Or alternatively:
 *
 *   gss_cred_id_t cred = [cuiCred acquireGSSCredential:&error];
 *
 * (They do exactly the same thing. One returns an autoreleased object, the other doesn't.)
 *
 * In the future the latter API will move into CredUI.
 */

@end
