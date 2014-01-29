//
//  CUIProxyCredential.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

__attribute__((visibility("default")))
@interface CUIProxyCredential : CUICredential <NSSecureCoding>
{
    NSMutableDictionary *_attributes;
    NSSet *_whitelist;
}

@property(nonatomic, readonly) NSSet *whitelist;

- (instancetype)initWithAttributes:(NSDictionary *)credential mutable:(BOOL)isMutable;
- (instancetype)initWithCredential:(CUICredential *)credential whitelist:(NSSet *)whitelist mutable:(BOOL)isMutable;

@end