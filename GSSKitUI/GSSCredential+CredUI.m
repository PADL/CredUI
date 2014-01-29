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

    cred = [cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrGSSCredential];
#if !__has_feature(objc_arc)
    [cred retain];
#endif
    
    if (cred == nil) {
        GSSItem *gssItem = [cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrGSSItem];
        if (gssItem) {
            cred = [gssItem acquire:[cuiCredential attributesWithClass:CUIAttributeClassGSSItem] error:error];
    #if !__has_feature(objc_arc)
            [cred retain];
    #endif
        }
    }

    if (cred == nil) {
        CFUUIDRef uuid = (__bridge CFUUIDRef)[cuiCredential.attributes objectForKey:(__bridge NSString *)kCUIAttrUUID];
        
        if (uuid)
            cred = (__bridge GSSCredential *)GSSCreateCredentialFromUUID(uuid);
    }

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