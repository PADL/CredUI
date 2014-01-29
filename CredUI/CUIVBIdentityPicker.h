//
//  CUIVBIdentityPicker.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <ViewBridge/ViewBridge.h>

/* Remote bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyConfigOptions;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyTitle;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyMessage;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyAttributes;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyAuthError;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyTargetName;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration;

/* Phased bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyPersist;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyGSSExportedContext;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyPAMSerializedHandle;

/* Service bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyReturnCode;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyLastError;
extern NSString * const _CUIIdentityPickerServiceBridgeKeySelectedCredential;

@protocol CUIIdentityPickerProxyProtocol
@end

@class NSRemoteView;

__attribute__((visibility("default")))
@interface CUIVBIdentityPicker : CUIIdentityPicker <NSViewBridgeKVOBuddy, NSRemoteViewDelegate>
{
    CUIUsageScenario _usageScenario;
    NSPanel *_containingPanel;
    NSRemoteView *_remoteView;
    const void *_context;
}

@property(nonatomic, retain) NSPanel *containingPanel;
@property(nonatomic, retain) NSRemoteView *remoteView;

+ (NSData *)exportGSSSecContext:(const void *)context;
+ (void *)importGSSSecContext:(NSData *)data;

@end
