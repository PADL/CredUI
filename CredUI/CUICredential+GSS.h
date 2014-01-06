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
 * [cred attributesWithClass:CUIAttributeClassGSSInitialCred).
 *
 * Instead, you need to use GSSKit, but at this stage I wanted to avoid creating a
 * dependency on that from CredUI. There is a separate framework, GSSKitUI, on which
 * you can use [GSSCredential initWithCUICredential:cred].
 */
@end
