//
//  CUICredential+GSSKitUI.m
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential (GSSKitUI)
- (gss_cred_id_t)acquireGSSCredential:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED
{
    GSSCredential *cred;
    
    cred = [[GSSCredential alloc] initWithCUICredential:self error:error];
    
    return (gss_cred_id_t)CFBridgingRetain(cred);
}
@end

