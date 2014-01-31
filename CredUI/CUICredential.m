//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#pragma mark - CUICFCredential concrete class

@implementation CUICFCredential

+ (id)allocWithZone:(NSZone *)zone
{
    static CUICredentialRef placeholderCred;
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (placeholderCred == NULL)
            placeholderCred = CUICredentialCreate(kCFAllocatorDefault, NULL);
    });
    
    return (id)placeholderCred;
}

CF_CLASSIMPLEMENTATION(CUICFCredential)

+ (BOOL)automaticallyNotifiesObserversForKey:(NSString *)theKey
{
    return NO;
}

- init
{
    NSAssert(0, @"CUICFCredential can only be instantiated through CredUICore");
    return nil;
}

- (NSDictionary *)attributes
{
    return (NSDictionary *)CUICredentialGetAttributes([self _credentialRef]);
}

- (void)didBecomeSelected
{
    CUICredentialDidBecomeSelected([self _credentialRef]);
}

- (void)didBecomeDeselected
{
    CUICredentialDidBecomeDeselected([self _credentialRef]);
}

- (BOOL)canSubmit
{
    return CUICredentialCanSubmit([self _credentialRef]);
}

- (void)willSubmit
{
    CUICredentialWillSubmit([self _credentialRef]);
}

- (void)didSubmit
{
    CUICredentialDidSubmit([self _credentialRef]);
}

- (void)savePersisted:(void (^)(NSError *))replyBlock
{
    return CUICredentialSavePersisted([self _credentialRef], (void (^)(CFErrorRef))replyBlock);
}

- (void)deletePersisted:(void (^)(NSError *))replyBlock
{
    return CUICredentialDeletePersisted([self _credentialRef], (void (^)(CFErrorRef))replyBlock);
}

- (NSArray *)fields
{
    return (NSArray *)CUICredentialGetFields([self _credentialRef]);
}

@end

#pragma mark - CUICredential abstract class

@implementation CUICredential

- (CUICredentialRef)_credentialRef
{
    return (CUICredentialRef)self;
}

- (CFTypeID)_cfTypeID
{
    return CUICredentialGetTypeID();
}

#pragma mark Primitive methods

- (NSDictionary *)attributes
{
    NSRequestConcreteImplementation(self, _cmd, [CUICredential class]);
    return nil;
}

- (NSArray *)fields
{
    NSRequestConcreteImplementation(self, _cmd, [CUICredential class]);
    return nil;
}

- (BOOL)canSubmit
{
    NSRequestConcreteImplementation(self, _cmd, [CUICredential class]);
    return NO;
}

#pragma mark Default methods

- (void)savePersisted:(void (^)(NSError *))replyBlock
{
    replyBlock(nil);
}

- (void)deletePersisted:(void (^)(NSError *))replyBlock
{
    replyBlock(nil);
}

- (void)didBecomeSelected
{
}

- (void)didBecomeDeselected
{
}

- (void)willSubmit
{
}

- (void)didSubmit
{
}

- (NSUUID *)UUID
{
    return [[self class] NSUUIDFromCFUUID:(__bridge CFUUIDRef)[self.attributes objectForKey:(__bridge id)kCUIAttrUUID]];
}

@end
