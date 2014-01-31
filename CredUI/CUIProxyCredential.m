//
//  CUIProxyCredential.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import "CUIProxyCredential.h"

@interface CUIProxyCredential ()
@property(nonatomic, retain) NSDictionary *attributes;
@property(nonatomic, retain) NSSet *whitelist;
@end

@implementation CUIProxyCredential

@synthesize attributes = _attributes;
@synthesize whitelist = _whitelist;
@synthesize identityPicker = _identityPicker;

- (instancetype)initWithAttributes:(NSDictionary *)attributes mutable:(BOOL)isMutable
{
    NSDictionary *tmpAttributes;

    if ((self = [super init]) == nil)
        return nil;

    tmpAttributes = isMutable ? [attributes mutableCopy] : [attributes copy];
    
    self.attributes = tmpAttributes;
    self.whitelist = nil;
 
#if !__has_feature(objc_arc)
    [tmpAttributes release];
#endif

    return self;
}

- (instancetype)initWithCredential:(CUICredential *)credential whitelist:(NSSet *)whitelist mutable:(BOOL)isMutable
{
    if ((self = [self initWithAttributes:credential.attributes mutable:isMutable]) == nil)
        return nil;
    
    self.whitelist = whitelist;
 
    return self;
}

- (void)dealloc
{
#if !__has_feature(objc_arc)
    [_attributes release];
    [_whitelist release];
    [_identityPicker release];
    [super dealloc];
#endif
}

- (NSArray *)fields
{
    return nil;
}

- (BOOL)canSubmit
{
    return YES;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (BOOL)isWhitelistedAttributeKey:(NSString *)key
{
    if (self.whitelist)
        return [self.whitelist containsObject:key];
    else
        return [super isWhitelistedAttributeKey:key];
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<CUIProxyCredential %p{name = \"%@\"}>",
                                      self,
                                      [self.attributes objectForKey:(__bridge id)kCUIAttrName]];
}

@end
