//
//  GSSCredential+CredUI.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation GSSCredential (CredUI)
- (instancetype)initWithCUICredential:(CUICredential *)cuiCredential error:(NSError **)error
{
    GSSCredential *cred = nil;
    GSSItem *gssItem;
    GSSName *name;
    
    /*
     * If a GSSItem tile was selected, we do not have access to the password, so
     * we must acquire it via the GSSItem API.
     */
    gssItem = cuiCredential.attributes[(__bridge NSString *)kCUIAttrGSSItem];
    if (gssItem) {
        cred = [gssItem acquire:[cuiCredential attributesWithClass:CUIAttributeClassGSSItem] error:error];
#if !__has_feature(objc_arc)
        [cred retain];
#endif
    } else {
        name = CFBridgingRelease([cuiCredential copyGSSName]);
        
        if (name) {
            cred = [self initWithName:name
                            mechanism:[GSSMechanism mechanismForCUICredential:cuiCredential]
                           attributes:[cuiCredential attributesWithClass:CUIAttributeClassGSSInitialCred]
                                error:error];
        }
    }
    
    return cred;
}
@end