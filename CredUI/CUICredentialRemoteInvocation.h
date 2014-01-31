//
//  CUICredentialRemoteInvocation.h
//  CredUI
//
//  Created by Luke Howard on 31/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@class CUIProxyCredential;
@class NSUUID;
@class NSError;

@interface CUICredentialRemoteInvocation : NSObject <NSSecureCoding>
{
    NSUUID *_invocationID;
    NSUUID *_credentialID;
    NSString *_selector;
    NSError *_error;
}

@property(nonatomic, assign) BOOL isReply;
@property(nonatomic, readonly) NSUUID *invocationID;
@property(nonatomic, retain) NSUUID *credentialID;
@property(nonatomic, retain) NSString *selector;
@property(nonatomic, retain) NSError *error;

@end
