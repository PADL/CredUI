//
//  CUICredential+GSSKitUI.h
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

/*
 * See comment in CUICredential+GSS.h Eventually this belongs in CredUI itself.
 */

@interface CUICredential (GSSKitUI)
- (gss_cred_id_t)acquireGSSCredential:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED;
@end
