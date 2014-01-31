//
//  CUICredential+Coding.m
//  CredUI
//
//  Created by Luke Howard on 28/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

//#define CUI_DYNAMIC_CLASSES 1

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

        builtinClasses = [[NSMutableSet alloc] initWithObjects:[NSArray class],
                                                               [NSData class],
                                                               [NSURL class],
                                                               [NSDate class],
                                                               [NSNumber class],
                                                               [NSError class],
                                                               [NSString class],
                                                               [NSDictionary class],
                                                               [NSUUID class],
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
    NSMutableDictionary *codeableAttrs = [[NSMutableDictionary alloc] init];
#if CUI_DYNAMIC_CLASSES
    NSMutableSet *codeableClassNames = [[NSMutableSet alloc] init];
    
    [[[self class] builtinClasses] enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        [codeableClassNames addObject:NSStringFromClass(obj)];
    }];

    [coder encodeObject:codeableClassNames forKey:@"classes"];
# if !__has_feature(objc_arc)
    [codeableClassNames release];
# endif
#endif /* CUI_DYNAMIC_CLASSES */

    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        BOOL bWhitelisted, bCodeable;
        
        /* allow passwords through gate for generic credentials */
        if ([key isEqual:(__bridge id)kCUIAttrCredentialPassword])
            bWhitelisted = bGenericCreds;
        else
            bWhitelisted = [self isWhitelistedAttributeKey:key];

#if CUI_DYNAMIC_CLASSES
        if (![[self class] isBuiltinClass:[obj class]])
            [codeableClassNames addObject:NSStringFromClass([obj class])];
#endif
        
        bCodeable = [obj conformsToProtocol:@protocol(NSSecureCoding)];
       
        /* CFUUIDRef/NSUUID are not bridged, so transform first */ 
        if (CFGetTypeID((__bridge CFTypeRef)obj) == CFUUIDGetTypeID()) {
            obj = [[self class] NSUUIDFromCFUUID:(__bridge CFUUIDRef)obj];
            bCodeable = YES;
        }
        
        if (bWhitelisted && bCodeable)
            [codeableAttrs setObject:obj forKey:key];
    }];
    
    [coder encodeObject:codeableAttrs forKey:@"attributes"];
#if !__has_feature(objc_arc)
    [codeableAttrs release];
#endif
}

- (id)initWithCoder:(NSCoder *)coder
{
    NSDictionary *credAttributes;
    NSMutableDictionary *transformedCredAttributes;
    id credential;

#if CUI_DYNAMIC_CLASSES
    NSSet *codeableClassNames = [coder decodeObjectOfClasses:[[self class] builtinClasses] forKey:@"classes"];
    NSMutableSet *codeableClasses = [[NSMutableSet alloc] init];
    [codeableClasses addObject:[NSDictionary class]];
    
    [codeableClassNames enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
        Class class = NSClassFromString(obj);
        if (class)
            [codeableClasses addObject:class];
    }];
# if !__has_feature(objc_arc)
    [codeableClasses autorelease];
# endif
#else
    NSSet *codeableClasses = [[self class] builtinClasses];
#endif /* CUI_DYNAMIC_CLASSES */
 
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
#endif
    
    return credential;
}

@end
