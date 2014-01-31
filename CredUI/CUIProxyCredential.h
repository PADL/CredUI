//
//  CUIProxyCredential.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@protocol CUIProxyCredentialRemoteInvocation
- (void)savePersisted:(void (^)(NSError *))replyBlock;
- (void)deletePersisted:(void (^)(NSError *))replyBlock;
@end

@class CUIVBIdentityPicker;

__attribute__((visibility("default")))
@interface CUIProxyCredential : CUICredential <NSSecureCoding, CUIProxyCredentialRemoteInvocation>
{
    NSMutableDictionary *_attributes;
    NSSet *_whitelist;
    CUIVBIdentityPicker *_identityPicker;
}

@property(nonatomic, readonly) NSSet *whitelist;
@property(nonatomic, retain) CUIVBIdentityPicker *identityPicker;

- (instancetype)initWithAttributes:(NSDictionary *)credential mutable:(BOOL)isMutable;
- (instancetype)initWithCredential:(CUICredential *)credential whitelist:(NSSet *)whitelist mutable:(BOOL)isMutable;

@end
