//
//  CUIIdentityPickerService.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <ViewBridge/ViewBridge.h>

@class CUIIdentityPickerInternal;

@interface CUIIdentityPickerService : NSViewService
{
    CUIIdentityPickerInternal *_identityPicker;
}

@property(nonatomic, retain) CUIIdentityPickerInternal *identityPicker;

@end