//
//  CUICredential+GSS.m
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "GSSItem.h"

@implementation CUICredential (GSS)

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
            if ([key isEqualToString:(__bridge NSString *)kCUIAttrCredentialSecIdentity] ||
                [key isEqualToString:(__bridge NSString *)kCUIAttrCredentialSecCertificate])
                transformedKey = (__bridge id)kGSSICCertificate; // special case
            else
                transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUIAttrCredential" withString:@"kGSSIC"];
        } else {
            transformedKey = key;
        }
        
        if (transformedKey)
            transformedDict[transformedKey] = obj;
    }];
    
    if (attrClass == CUIAttributeClassGSSInitialCred) {
        // we only emit initiator creds
        transformedDict[(NSString *)kGSSCredentialUsage] = (NSString *)kGSS_C_INITIATE;
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
    
    item = self.attributes[(__bridge NSString *)kCUIAttrGSSItem];
    if (item == nil) {
        NSDictionary *itemAttrs = [self attributesWithClass:CUIAttributeClassGSSItem];
        NSArray *matchingItems = CFBridgingRelease(GSSItemCopyMatching((__bridge CFDictionaryRef)itemAttrs, NULL));
        
        item = [matchingItems firstObject];
        if (item == nil && addIfAbsent)
            item = CFBridgingRelease(GSSItemAdd((__bridge CFDictionaryRef)itemAttrs, error ? &cfError : NULL));
    }
    
    if (error)
        *error = CFBridgingRelease(cfError);
    
    return (GSSItemRef)CFBridgingRetain(item);
}

- (gss_name_t)copyGSSName CF_RETURNS_RETAINED
{
    gss_name_t name = GSS_C_NO_NAME;
    gss_const_OID oid = GSS_C_NO_OID;
    
    id type = self.attributes[(__bridge NSString *)kCUIAttrNameType];
    id value = self.attributes[(__bridge NSString *)kCUIAttrName];
    
    if (type == nil ||
        [type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSUsername])
        oid = GSS_C_NT_USER_NAME;
    else if ([type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSHostBasedService])
        oid = GSS_C_NT_HOSTBASED_SERVICE;
    else if ([type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSExportedName])
        oid = GSS_C_NT_EXPORT_NAME;
    
    if (oid != GSS_C_NO_OID && value != nil)
        name = GSSCreateName((__bridge CFStringRef)value, oid, NULL);
    
    return name;
}

@end
