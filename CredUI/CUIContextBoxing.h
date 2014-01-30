//
//  CUIContextBoxing.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CUIIdentityPicker.h>

@protocol CUIContextBoxing <NSObject>
@property (nonatomic, assign) void *context;

- (NSData *)exportContext;
- (BOOL)importContext:(NSData *)data;

@end

@interface CUIIdentityPicker (CUIContextBoxing)
@property (nonatomic, retain) id <CUIContextBoxing> contextBox;
@end

