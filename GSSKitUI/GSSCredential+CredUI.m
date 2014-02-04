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

    /*
     * If the credential was already cached (e.g. by the IdentityPickerService), then return it.
     */
    cred = [cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrGSSCredential];
#if !__has_feature(objc_arc)
    [cred retain];
#endif
   
    /*
     * If the credential came from the GSSItem credential provider, then use that interface to
     * acquire a gss_cred_id_t.
     */ 
    if (cred == nil) {
        GSSItem *gssItem = [cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrGSSItem];
        if (gssItem) {
            cred = [gssItem acquire:[cuiCredential attributesWithClass:CUIAttributeClassGSSItem] error:error];
    #if !__has_feature(objc_arc)
            [cred retain];
    #endif
        }
    }

    /*
     * If the credential has a UUID reference, then use GSSCreateCredentialFromUUID().
     */
    if (cred == nil) {
        CFUUIDRef uuid = (__bridge CFUUIDRef)[cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrUUID];
        
        if (uuid)
            cred = (__bridge GSSCredential *)GSSCreateCredentialFromUUID(uuid);
    }

    /*
     * Otherwise, hope we have the necessary attributes to acquire a credential directly using
     * gss_aapl_initial_cred() or some variant thereof.
     */
    if (cred == nil) {
        gss_name_t name = [cuiCredential copyGSSName];
        if (name) {
            cred = [self initWithName:(__bridge GSSName *)name
                            mechanism:[GSSMechanism mechanismForCUICredential:cuiCredential]
                           attributes:[cuiCredential attributesWithClass:CUIAttributeClassGSSInitialCred]
                                error:error];
            CFRelease(name);
        }
    }
    
    return cred;
}

@end
