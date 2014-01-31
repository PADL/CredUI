//
//  CUIVBIdentityPicker.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <ViewBridge/ViewBridge.h>
#import <CredUI/CUIContextBoxing.h>

/* Remote bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyConfigOptions;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyTitle;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyMessage;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyAttributes;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyAuthError;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyTargetName;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyStartCredentialEnumeration;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyInvocation;

/* Phased bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyPersist;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyExportedContext;

/* Service bridge keys */
extern NSString * const _CUIIdentityPickerServiceBridgeKeyReturnCode;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyLastError;
extern NSString * const _CUIIdentityPickerServiceBridgeKeySelectedCredential;
extern NSString * const _CUIIdentityPickerServiceBridgeKeyInvocationReply;

@protocol CUIIdentityPickerProxyProtocol
@end

@class NSRemoteView;
@class CUIProxyCredential;

@interface CUIVBIdentityPicker : CUIIdentityPicker <NSViewBridgeKVOBuddy, NSRemoteViewDelegate>
{
    CUIUsageScenario _usageScenario;
    NSPanel *_containingPanel;
    NSRemoteView *_remoteView;
    id <CUIContextBoxing> _contextBox;
    NSMutableDictionary *_invocationReplyDict;
}

@property(nonatomic, retain) NSPanel *containingPanel;
@property(nonatomic, retain) NSRemoteView *remoteView;
@property(nonatomic, retain) id <CUIContextBoxing> contextBox;
@property(nonatomic, retain) NSMutableDictionary *invocationReplyDict;

- (void)credentialInvocation:(CUIProxyCredential *)credential
                    selector:(NSString *)selector
                   withReply:(void (^)(NSError *))replyBlock;

@end
