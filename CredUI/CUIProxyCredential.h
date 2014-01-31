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

__attribute__((visibility("default")))
@interface CUIProxyCredential : CUICredential <NSSecureCoding, CUIProxyCredentialRemoteInvocation>
{
    NSMutableDictionary *_attributes;
    NSSet *_whitelist;
    CUIIdentityPicker *_identityPicker;
}

@property(nonatomic, readonly) NSSet *whitelist;
@property(nonatomic, retain) CUIIdentityPicker *identityPicker;

- (instancetype)initWithAttributes:(NSDictionary *)credential mutable:(BOOL)isMutable;
- (instancetype)initWithCredential:(CUICredential *)credential whitelist:(NSSet *)whitelist mutable:(BOOL)isMutable;

@end
