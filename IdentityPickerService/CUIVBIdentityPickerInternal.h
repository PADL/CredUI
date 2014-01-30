//
//  CUIVBIdentityPickerInternal
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <CredUI/CredUI.h>
#import <CredUI/CUIVBIdentityPicker.h>

#import <CredUICore/CredUICore.h>

#import <CredUI/CUIIdentityPickerInternal.h>

#import "CUIIdentityPickerListenerDelegate.h"
#import "CUIIdentityPickerService.h"

@class NSViewBridge;

@interface CUIVBIdentityPickerInternal : CUIIdentityPickerInternal
{
    NSViewBridge *_bridge;
    void *_context;
}

- (instancetype)initWithViewBridge:(NSViewBridge *)viewBridge;
- (void)setGSSExportedContext:(NSData *)contextData;
- (void)setPAMSerializedHandle:(NSData *)handleData;

@property(nonatomic, retain) NSViewBridge *bridge;

@end
