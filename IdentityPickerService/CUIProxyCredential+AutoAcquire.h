//
//  CUICredential+AutoAcquire.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/CUIProxyCredential.h>

@interface CUIProxyCredential (AutoAcquire)

/* Acquire GSS cred and set it in attributes */
- (BOOL)autoAcquireGSSCred:(NSError * __autoreleasing *)error;

@end
