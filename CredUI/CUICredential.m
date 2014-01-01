//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CFBridgeHelper.h"

@interface CUICFCredential : CUICredential
@end

@implementation CUICFCredential

+ (void)load
{
    _CFRuntimeBridgeClasses(CUICredentialGetTypeID(), "CUICFCredential");
}

+ (id)allocWithZone:(NSZone *)zone
{
    static CUICFCredential *placeholderCred;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (placeholderCred == nil)
            placeholderCred = [super allocWithZone:zone];
    });

    return placeholderCred;
}

CF_CLASSIMPLEMENTATION(CUICFCredential)

- (instancetype)initWithContext:(IUnknownVTbl *)context
{
    CUICredentialRef credentialRef = CUICredentialCreate(kCFAllocatorDefault, context);
    
    self = (id)credentialRef;
    
    return NSMakeCollectable(self);
}

@end

@implementation CUICredential

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (CUICredentialRef)_credentialRef
{
    return (CUICredentialRef)self;
}

- (NSString *)description
{
    return [NSMakeCollectable(CFCopyDescription([self _credentialRef])) autorelease];
}

- (CFTypeID)_cfTypeID
{
    return CUIFieldGetTypeID();
}

- (instancetype)init
{
    return [self initWithContext:NULL];
}

- (instancetype)initWithContext:(IUnknownVTbl *)context
{
    return [super init];
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.attributes];
}

- (id)initWithCoder:(NSCoder *)coder
{
    NSDictionary *credAttributes;
    CUICredentialRef credentialRef;
    
    credAttributes = [coder decodeObject];
    if (credAttributes == nil)
        return nil;
        
    credentialRef = CUICredentialCreateProxy(kCFAllocatorDefault, (CFDictionaryRef)credAttributes);
    
    return (id)credentialRef;
}

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass
{
    return (CUIField *)CUICredentialFindFirstFieldWithClass([self _credentialRef], fieldClass);
}

- (NSDictionary *)attributes
{
    return (NSDictionary *)CUICredentialGetAttributes([self _credentialRef]);
}

- (void)didBecomeSelected:(BOOL *)pbAutoLogin
{
    Boolean bAutoLogin = false;
    
    CUICredentialDidBecomeSelected([self _credentialRef], &bAutoLogin);
    
    *pbAutoLogin = bAutoLogin;
}

- (void)didBecomeDeselected
{
    CUICredentialDidBecomeDeselected([self _credentialRef]);
}

- (BOOL)canSubmit
{
    return CUICredentialCanSubmit([self _credentialRef]);
}

- (void)willSubmit
{
    CUICredentialWillSubmit([self _credentialRef]);
}

- (void)didSubmit
{
    CUICredentialDidSubmit([self _credentialRef]);
}

- (GSSItem *)GSSItem
{
    NSDictionary *itemAttrs = [self attributesWithClass:CUIAttributeClassGSSItem];
    NSArray *matchingItems = [NSMakeCollectable(GSSItemCopyMatching((CFDictionaryRef)itemAttrs, NULL)) autorelease];

    return matchingItems.count ? matchingItems[0] : nil;
}

- (id)GSSName
{
    gss_name_t name = GSS_C_NO_NAME;
    gss_const_OID oid = GSS_C_NO_OID;

    id type = self.attributes[(NSString *)kCUIAttrNameType];
    id value = self.attributes[(NSString *)kCUIAttrName];

    if ([type isEqual:(NSString *)kCUIAttrNameTypeGSSUsername])
        oid = GSS_C_NT_USER_NAME;
    else if ([type isEqual:(NSString *)kCUIAttrNameTypeGSSHostBasedService])
        oid = GSS_C_NT_HOSTBASED_SERVICE;
    else if ([type isEqual:(NSString *)kCUIAttrNameTypeGSSExportedName])
        oid = GSS_C_NT_EXPORT_NAME;
    
    if (oid != GSS_C_NO_OID)
        name = GSSCreateName(value, oid, NULL);
    
    return [NSMakeCollectable(name) autorelease];
}

- (NSDictionary *)attributesWithClass:(CUIAttributeClass)attrClass
{
    NSMutableDictionary *transformedDict = [NSMutableDictionary dictionary];

    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSString *transformedKey;

        if (attrClass == CUIAttributeClassGSSItem)
            transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUI" withString:@"kGSS"];
        else if (attrClass == CUIAttributeClassGSSInitialCred)
            transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUIAttrCredential" withString:@"kGSSIC"];
        else
            transformedKey = key;

        transformedDict[transformedKey] = obj;
    }];

    if (attrClass == CUIAttributeClassGSSInitialCred) {
        // we only emit initiator creds
        transformedDict[(NSString *)kGSSCredentialUsage] = (NSString *)kGSS_C_INITIATE;
    }
    
    return transformedDict;
}

- (BOOL)addGSSItem:(NSError * __autoreleasing *)error
{
    NSDictionary *itemAttributes = [self attributesWithClass:CUIAttributeClassGSSItem];
    GSSItemRef item;
    BOOL ret = NO;
    
    if (error)
        *error = nil;
    
    item = GSSItemAdd((CFDictionaryRef)itemAttributes, (CFErrorRef *)error);
    if (item) {
        CFRelease(item);
        ret = YES;
    }
    
    if (error)
        [NSMakeCollectable(*error) autorelease];
        
    return ret;
}

- (BOOL)confirm:(NSError * __autoreleasing *)error
{
    return [self addGSSItem:error];
}

- (NSArray *)fields
{
    return (NSArray *)CUICredentialGetFields([self _credentialRef]);
}

@end
