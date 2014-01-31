//
//  CUIIdentityPickerService.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <ViewBridge/ViewBridge.h>
#import <CredUI/CUIContextBoxing.h>

@class CUIVBIdentityPickerInternal;

@interface CUIIdentityPickerService : NSViewService <CUIContextBoxing>
{
    CUIVBIdentityPickerInternal *_identityPicker;
    void *_context;
}

@property(nonatomic, readonly) CUIUsageScenario usageScenario;
@property(nonatomic, retain) CUIVBIdentityPickerInternal *identityPicker;

@end
