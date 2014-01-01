//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential
{
}

#pragma mark Initialization

+ (void)load
{
    _CFRuntimeBridgeClasses(CUICredentialGetTypeID(), "CUICredential");
}

+ (id)allocWithZone:(NSZone *)zone
{
    return nil;
}

#pragma mark Bridging

- (id)retain
{
    return CFRetain((CUICredentialRef)self);
}

- (oneway void)release
{
    CFRelease((CUICredentialRef)self);
}

- (NSUInteger)retainCount
{
    return CFGetRetainCount((CUICredentialRef)self);
}

- (BOOL)isEqual:(id)anObject
{
    return (BOOL)CFEqual((CFTypeRef)self, (CFTypeRef)anObject);
}

- (NSUInteger)hash
{
    return CFHash((CFTypeRef)self);
}

- (NSString *)description
{
    CFStringRef copyDesc = CFCopyDescription((CFTypeRef)self);
    
    return CFBridgingRelease(copyDesc);
}

- (BOOL)allowsWeakReference
{
    return !_CFIsDeallocating(self);
}

- (BOOL)retainWeakReference
{
    return _CFTryRetain(self) != nil;
}

- (CFTypeID)_cfTypeID
{
    return CUIFieldGetTypeID();
}

- (CUICredentialRef)_credentialRef
{
    return (__bridge CUICredentialRef)self;
}

- (NSArray *)fields
{
    return (__bridge NSArray *)CUICredentialGetFields([self _credentialRef]);
}

- (instancetype)init
{
    return [self initWithContext:NULL];
}

- (instancetype)initWithContext:(IUnknownVTbl *)context
{
    CUICredentialRef credentialRef = CUICredentialCreate(kCFAllocatorDefault, context);
    
    self = (__bridge id)credentialRef;
    
    return self;
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
    
    credentialRef = CUICredentialCreateProxy(kCFAllocatorDefault, (__bridge CFDictionaryRef)credAttributes);
    
    self = (__bridge id)credentialRef;
    
    return self;
}

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass
{
    return (__bridge CUIField *)CUICredentialFindFirstFieldWithClass([self _credentialRef], fieldClass);
}

- (NSDictionary *)attributes
{
    return (__bridge NSDictionary *)CUICredentialGetAttributes([self _credentialRef]);
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

- (void)fieldsApplyBlock:(void (^)(CUIFieldRef, BOOL *))cb
{
    CUICredentialFieldsApplyBlock([self _credentialRef], ^(CUIFieldRef field, Boolean *pStop) {
        BOOL stop = *pStop;
        cb(field, &stop);
        *pStop = stop;
    });
}

- (GSSItem *)GSSItem
{
    NSDictionary *itemAttrs = [self attributesWithClass:CUIAttributeClassGSSItem];
    NSArray *matchingItems = CFBridgingRelease(GSSItemCopyMatching((__bridge CFDictionaryRef)itemAttrs, NULL));

    return matchingItems.count ? matchingItems[0] : nil;
}

- (id)GSSName
{
    gss_name_t name = GSS_C_NO_NAME;
    gss_const_OID oid = GSS_C_NO_OID;
    CFErrorRef error = NULL;

    id type = self.attributes[(__bridge NSString *)kCUIAttrNameType];
    id value = self.attributes[(__bridge NSString *)kCUIAttrName];

    if ([type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSUsername])
        oid = GSS_C_NT_USER_NAME;
    else if ([type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSHostBasedService])
        oid = GSS_C_NT_HOSTBASED_SERVICE;
    else if ([type isEqual:(__bridge NSString *)kCUIAttrNameTypeGSSExportedName])
        oid = GSS_C_NT_EXPORT_NAME;
    
    if (oid) {
        name = GSSCreateName(value, oid, &error);
        
        if (name)
            return CFBridgingRelease(name);
        else if (error)
            CFRelease(error);
    }
    
    return nil;
}

- (NSDictionary *)attributesWithClass:(CUIAttributeClass)attrClass
{
    NSMutableDictionary *transformedDict = [[NSMutableDictionary alloc] init];

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
        transformedDict[(__bridge NSString *)kGSSCredentialUsage] = (__bridge NSString *)kGSS_C_INITIATE;
    }
    
    return transformedDict;
}

- (BOOL)addGSSItem:(NSError * __autoreleasing *)error
{
    NSDictionary *itemAttributes = [self attributesWithClass:CUIAttributeClassGSSItem];
    GSSItemRef item;
    CFErrorRef cfError;
    BOOL ret = NO;
    
    if (error)
        *error = nil;
    
    item = GSSItemAdd((__bridge CFDictionaryRef)itemAttributes, &cfError);
    if (item) {
        ret = YES;
        CFRelease(item);
    }
    
    if (cfError) {
        if (error)
            *error = CFBridgingRelease(cfError);
        else
            CFRelease(cfError);
    }
    
    return ret;
}

@end
