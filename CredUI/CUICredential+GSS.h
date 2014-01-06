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
@end