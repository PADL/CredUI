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
 * yet have enough logic to support arbitrary credential dictionaries (although you
 * can acquire a password or certificate credential by passing it the result of calling
 * [cred attributesWithClass:CUIAttributeClassGSSInitialCred):
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
 * It is recommended instead that you use GSSKit. CredUI does not link against GSSKit
 * in order to minimize dependencies. However, there is a separate framework, GSSKitUI
 * which lets you do the following:
 *
 *   GSSCredential *cred = [GSSCredential intiWithCUICredential:cuiCred];
 *
 * That will also respect any mechanism selections that were made by the credential
 * provider, which the above example of gss_aapl_initial_cred does not do.
 */

#if 0
/* once gss_aapl_initial_cred() has been updated ... */
- (gss_cred_id_t)acquireGSSCredential CF_RETURNS_RETAINED;
#endif

@end
