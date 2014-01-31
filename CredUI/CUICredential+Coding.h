//
//  CUICredential+Coding.h
//  CredUI
//
//  Created by Luke Howard on 28/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUICredential (Coding) <NSSecureCoding>

- (BOOL)isWhitelistedAttributeKey:(NSString *)key;

+ (id)CFUUIDFromNSUUID:(NSUUID *)nsUuid;
+ (NSUUID *)NSUUIDFromCFUUID:(CFUUIDRef)cfUuid;

@end
