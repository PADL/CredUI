//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CFBridgeHelper.h"
#include "CUIProviderUtilities.h"

#pragma mark - CUICFCredential concrete class

@interface CUICFCredential : CUICredential
@end

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

- (NSDictionary *)attributes
{
    return (NSDictionary *)CUICredentialGetAttributes([self _credentialRef]);
}

- (void)didBecomeSelected:(BOOL *)pbAutoLogin
{
    Boolean bAutoLogin = false;
    
    CUICredentialDidBecomeSelected([self _credentialRef], &bAutoLogin);
    
    *pbAutoLogin = bAutoLogin;
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

- (BOOL)savePersisted:(NSError * __autoreleasing *)error
{
    BOOL ret;
    
    if (error)
        *error = nil;
    
    ret = CUICredentialSavePersisted([self _credentialRef], (CFErrorRef *)error);
    if (error)
        [NSMakeCollectable(*error) autorelease];
    
    return ret;
}

- (BOOL)deletePersisted:(NSError * __autoreleasing *)error
{
    BOOL ret;
    
    if (error)
        *error = nil;
    
    ret = CUICredentialDeletePersisted([self _credentialRef], (CFErrorRef *)error);
    if (error)
        [NSMakeCollectable(*error) autorelease];
    
    return ret;
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

#pragma mark Secure Coding

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    NSSet *codeableKeys = [self.attributes keysOfEntriesPassingTest:^ BOOL (id key, id obj, BOOL *stop) {
        return [obj respondsToSelector:@selector(encodeWithCoder:)];
    }];
    
    NSDictionary *codeableAttrs =
        [[NSDictionary alloc] initWithObjects:[self.attributes objectsForKeys:codeableKeys.allObjects notFoundMarker:[NSNull null]]
                                      forKeys:codeableKeys.allObjects];
    [coder encodeObject:codeableAttrs forKey:@"attributes"];
    
    [codeableAttrs release];
}

- (id)initWithCoder:(NSCoder *)coder
{
    NSDictionary *credAttributes;
    CUICredentialRef credentialRef;
    
    credAttributes = [coder decodeObjectOfClass:[NSDictionary class] forKey:@"attributes"];
    if (credAttributes == nil)
        return nil;
        
    credentialRef = CUICredentialCreateProxy(kCFAllocatorDefault, (CFDictionaryRef)credAttributes);
    
    return (id)credentialRef;
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

- (BOOL)savePersisted:(NSError * __autoreleasing *)error
{
    return NO;
}

- (BOOL)deletePersisted:(NSError * __autoreleasing *)error
{
    return NO;
}

- (void)didBecomeSelected:(BOOL *)pbAutoLogin
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

- (NSDictionary *)attributesWithClass:(CUIAttributeClass)attrClass
{
    NSMutableDictionary *transformedDict = [NSMutableDictionary dictionary];
    
    [self.attributes enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL *stop) {
        NSString *transformedKey = nil;
        
        if (attrClass == CUIAttributeClassGSSItem) {
            if ([key isEqualToString:(__bridge NSString *)kCUIAttrCredentialPassword] && [obj boolValue])
                transformedKey = nil; /* remove placeholder passwords */
            else
                transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUI" withString:@"kGSS"];
        } else if (attrClass == CUIAttributeClassGSSInitialCred) {
            if ([key isEqualToString:(__bridge NSString *)kCUIAttrCredentialSecIdentity])
                transformedKey = (__bridge id)kGSSICCertificate; // special case
            else
                transformedKey = [key stringByReplacingOccurrencesOfString:@"kCUIAttrCredential" withString:@"kGSSIC"];
        } else {
            transformedKey = key;
        }
        
        if (transformedKey)
            transformedDict[transformedKey] = obj;
    }];
    
    if (attrClass == CUIAttributeClassGSSInitialCred) {
        // we only emit initiator creds
        transformedDict[(NSString *)kGSSCredentialUsage] = (NSString *)kGSS_C_INITIATE;
    }
    
    return transformedDict;
}

@end