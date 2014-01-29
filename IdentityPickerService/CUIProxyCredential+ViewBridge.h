//
//  CUICredential+ViewBridge.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/CUIProxyCredential.h>

@interface CUIProxyCredential (ViewBridge)

/* Acquire GSS cred and set it in attributes */
- (BOOL)acquireAndSetGSSCredential;
- (BOOL)authAndSetAuthenticatedForLoginScenario;

@end
