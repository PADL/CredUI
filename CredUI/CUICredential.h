//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class NSDictionary;
@class GSSItem;

typedef NS_ENUM(NSUInteger, CUIAttributeClass) {
    CUIAttributeClassGeneric                = 1,    // generic credential attributes
    CUIAttributeClassGSSInitialCred         = 2,    // attributes for gss_aapl_initial_cred
    CUIAttributeClassGSSItem                = 4     // attributes for GSSItemAdd
};

@interface CUICredential : NSObject

- (NSDictionary *)attributes;
- (NSDictionary *)attributesWithClass:(CUIAttributeClass)flags;

- (GSSItem *)GSSItem;
- (id)GSSName;

@end
