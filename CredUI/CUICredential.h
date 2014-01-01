//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class NSDictionary;
@class GSSItem;

@interface CUICredential : NSObject

- (NSDictionary *)attributes;
- (NSDictionary *)attributesWithClass:(CUIAttributeClass)flags;

- (GSSItem *)GSSItem;
- (id)GSSName;

@end
