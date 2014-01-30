//
//  CUICredential+GSSKitUI.m
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential (GSSKitUI)

- (NSDictionary *)attributesWithClass:(CUIAttributeClass)attrClass
{
    NSMutableDictionary *transformedDict = [NSMutableDictionary dictionary];
    
    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSString *transformedKey = nil;
        
        if (attrClass == CUIAttributeClassGSSItem) {
            if ([key isEqualToString:(__bridge NSString *)kCUIAttrCredentialPassword] && [obj boolValue])
                transformedKey = nil; /* remove placeholder passwords */
            else
                transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUI" withString:@"kGSS"];
        } else if (attrClass == CUIAttributeClassGSSInitialCred) {
            if ([key isEqualToString:(__bridge NSString *)kCUIAttrCredentialSecIdentity]
#if 0
                || [key isEqualToString:(__bridge NSString *)kCUIAttrCredentialSecCertificate]
#endif
                )
                transformedKey = (__bridge id)kGSSICCertificate; // special case
            else
                transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUIAttrCredential" withString:@"kGSSIC"];
        } else {
            transformedKey = key;
        }
        
        if (transformedKey)
            [transformedDict setObject:obj forKey:transformedKey];
    }];
    
    if (attrClass == CUIAttributeClassGSSInitialCred) {
        // we only emit initiator creds
        [transformedDict setObject:(NSString *)kGSS_C_INITIATE forKey:(NSString *)kGSSCredentialUsage];
    }
    
    return transformedDict;
}

/*
 * GSS item for the credential.
 */
- (GSSItemRef)copyMatchingGSSItem:(BOOL)addIfAbsent error:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED
{
    id item;
    CFErrorRef cfError = NULL;
    
    if (error)
        *error = nil;
    
    item = [self.attributes objectForKey:(__bridge NSString *)kCUIAttrGSSItem];
    if (item == nil) {
        NSDictionary *itemAttrs = [self attributesWithClass:CUIAttributeClassGSSItem];
        NSArray *matchingItems = CFBridgingRelease(GSSItemCopyMatching((__bridge CFDictionaryRef)itemAttrs, NULL));
        
        item = [matchingItems firstObject];
        if (item == nil && addIfAbsent)
            item = CFBridgingRelease(GSSItemAdd((__bridge CFDictionaryRef)itemAttrs, error ? &cfError : NULL));
    }
    
    if (error && cfError)
        *error = CFBridgingRelease(cfError);
    
    return (GSSItemRef)CFBridgingRetain(item);
}

- (gss_name_t)copyGSSName CF_RETURNS_RETAINED
{
    return CUICopyGSSNameForAttributes((__bridge CFDictionaryRef)self.attributes);
}

- (gss_cred_id_t)acquireGSSCredential:(NSError * __autoreleasing *)error CF_RETURNS_RETAINED
{
    GSSCredential *cred;
    
    cred = [[GSSCredential alloc] initWithCUICredential:self error:error];
    
    return (gss_cred_id_t)CFBridgingRetain(cred);
}

@end

