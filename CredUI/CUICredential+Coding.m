//
//  CUICredential+Coding.m
//  CredUI
//
//  Created by Luke Howard on 28/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential (Coding)

#pragma mark Secure Coding

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (BOOL)isWhitelistedAttributeKey:(NSString *)key
{
    /* This can be overriden by subclasses */
    return YES;
}

+ (NSSet *)builtinClasses
{
    static dispatch_once_t onceToken = 0;
    static NSMutableSet *builtinClasses = nil;
        
    dispatch_once(&onceToken, ^{
        static NSArray *gssClassNames;

        builtinClasses = [[NSMutableSet alloc] initWithObjects:[NSString class],
                                                            [NSNumber class],
                                                            [NSUUID class],
                                                            [NSArray class],
                                                            [NSDictionary class],
                                                            [NSSet class],
                                                            nil];

        /* these will only be available if GSSKit is linked */
        gssClassNames = [NSArray arrayWithObjects:@"GSSItem",
                                                  @"GSSCredential",
                                                  @"GSSName",
                                                  nil];
        [gssClassNames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            Class class = NSClassFromString(obj);
            if (obj)
                [builtinClasses addObject:class];

        }];
    });

    return builtinClasses;
}

+ (BOOL)isBuiltinClass:(id)aClass
{
    __block BOOL ret = NO;
    
    [[[self class] builtinClasses] enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        if ([aClass isSubclassOfClass:obj]) {
            *stop = ret = YES;
        }
    }];
    
    return ret;
}

+ (NSUUID *)NSUUIDFromCFUUID:(CFUUIDRef)cfUuid
{
    CFStringRef uuidString = CFUUIDCreateString(kCFAllocatorDefault, cfUuid);
    NSUUID *nsUuid = [[NSUUID alloc] initWithUUIDString:(__bridge NSString *)uuidString];
    
#if !__has_feature(objc_arc)
    [nsUuid autorelease];
#endif
    CFRelease(uuidString);
    
    return nsUuid;
}

+ (id)CFUUIDFromNSUUID:(NSUUID *)nsUuid
{
    CFUUIDRef cfUuid = CFUUIDCreateFromString(kCFAllocatorDefault, (__bridge CFStringRef)[nsUuid UUIDString]);
    if (cfUuid == NULL)
        return nil;
    
    return CFBridgingRelease(cfUuid);
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    BOOL bGenericCreds = [[self.attributes objectForKey:(__bridge id)kCUIAttrClass] isEqual:(__bridge id)kCUIAttrClassGeneric];
    NSMutableSet *codeableClassNames = [[NSMutableSet alloc] init];
    NSMutableDictionary *codeableAttrs = [[NSMutableDictionary alloc] init];
    
    [[[self class] builtinClasses] enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        [codeableClassNames addObject:NSStringFromClass(obj)];
    }];
    
    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        BOOL bWhitelisted, bCodeable;
        
        /* allow passwords through gate for generic credentials */
        if ([key isEqual:(__bridge id)kCUIAttrCredentialPassword])
            bWhitelisted = bGenericCreds;
        else
            bWhitelisted = [self isWhitelistedAttributeKey:key];
        
        if (![[self class] isBuiltinClass:[obj class]])
            [codeableClassNames addObject:NSStringFromClass([obj class])];
        
        bCodeable = [obj conformsToProtocol:@protocol(NSSecureCoding)];
       
        /* CFUUIDRef/NSUUID are not bridged, so transform first */ 
        if (CFGetTypeID((__bridge CFTypeRef)obj) == CFUUIDGetTypeID()) {
            obj = [[self class] NSUUIDFromCFUUID:(__bridge CFUUIDRef)obj];
            bCodeable = YES;
        }
        
        if (bWhitelisted && bCodeable)
            [codeableAttrs setObject:obj forKey:key];
    }];
    
    [coder encodeObject:codeableClassNames forKey:@"classes"];
    [coder encodeObject:codeableAttrs forKey:@"attributes"];
    
#if !__has_feature(objc_arc)
    [codeableAttrs release];
    [codeableClassNames release];
#endif
}

- (id)initWithCoder:(NSCoder *)coder
{
    NSDictionary *credAttributes;
    NSMutableDictionary *transformedCredAttributes;
    NSMutableSet *codeableClasses;
    id credential;

    // XXX is this insecure? should we restrict to builtin classes?    
    NSSet *codeableClassNames = [coder decodeObjectOfClasses:[[self class] builtinClasses] forKey:@"classes"];
    
    codeableClasses = [[NSMutableSet alloc] init];
    [codeableClasses addObject:[NSDictionary class]];
    
    [codeableClassNames enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        Class class = NSClassFromString(obj);
        if (class)
            [codeableClasses addObject:class];
    }];
    
    credAttributes = [coder decodeObjectOfClasses:codeableClasses forKey:@"attributes"];
    if (credAttributes == nil)
        return nil;
    
    transformedCredAttributes = [[NSMutableDictionary alloc] init];
    
    [credAttributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        if ([obj isKindOfClass:[NSUUID class]])
            [transformedCredAttributes setObject:[[self class] CFUUIDFromNSUUID:obj] forKey:key];
        else
            [transformedCredAttributes setObject:obj forKey:key];
    }];
    
    credential = [[CUIProxyCredential alloc] initWithAttributes:transformedCredAttributes mutable:NO];
    
#if !__has_feature(objc_arc)
    [transformedCredAttributes release];
    [codeableClasses release];
#endif
    
    return credential;
}

@end
