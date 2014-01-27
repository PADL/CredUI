//
//  CUIIdentityPickerInfo.h
//  CredUI
//
//  Created by Luke Howard on 28/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInfo : NSObject
{
    id _delegate;
    SEL _didEndSelector;
}
@property(nonatomic, retain) id delegate;
@property(nonatomic, assign) SEL didEndSelector;
@end
