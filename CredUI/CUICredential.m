//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential

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

- init
{
    return [self initWithContext:NULL];
}

- initWithContext:(IUnknownVTbl *)context
{
    CUICredentialRef credentialRef = CUICredentialCreate(kCFAllocatorDefault, context);
    
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
    Boolean bAutoLogin;
    CUICredentialDidBecomeSelected([self _credentialRef], &bAutoLogin);
    *pbAutoLogin = bAutoLogin;
}

- (void)didBecomeDeselected
{
    CUICredentialDidBecomeDeselected([self _credentialRef]);
}

- (void)willSubmit
{
    CUICredentialWillSubmit([self _credentialRef]);
}

- (void)didSubmit
{
    CUICredentialDidSubmit([self _credentialRef]);
}

#if 0
- (void)fieldsApplyBlock:(void (^)(CUIFieldRef, BOOL *))block
                    stop:(BOOL *)stop
{
}
#endif

- (GSSItem *)GSSItem
{
    return (__bridge GSSItem *)CUICredentialGetGSSItem([self _credentialRef]);
}

- (NSDictionary *)attributesWithDisposition:(CUIFlags)flags
{
    NSMutableDictionary *transformedDict = [[NSMutableDictionary alloc] init];

    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSString *transformedKey;

        if (flags & CUIFlagsGSSItemDisposition)
            transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUI" withString:@"kGSS"];
        else if (flags & CUIFlagsGSSAcquireCredsDisposition)
            transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUIAttrCredential" withString:@"kGSSIC"];
        else
            transformedKey = key;

        transformedDict[transformedKey] = obj;
    }];

    return transformedDict;
}

@end
