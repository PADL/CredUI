//
//  CUICredential.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredential

#pragma mark Initialization

+ (void)load
{
    _CFRuntimeBridgeClasses(CUICredentialGetTypeID(), "CUICredential");
}

+ (id)allocWithZone:(NSZone *)zone
{
    return nil;
}

#pragma mark Bridging

- (id)retain
{
    return CFRetain((GSSItemRef)self);
}

- (oneway void)release
{
    CFRelease((GSSItemRef)self);
}

- (NSUInteger)retainCount
{
    return CFGetRetainCount((GSSItemRef)self);
}

- (BOOL)isEqual:(id)anObject
{
    return (BOOL)CFEqual((CFTypeRef)self, (CFTypeRef)anObject);
}

- (NSUInteger)hash
{
    return CFHash((CFTypeRef)self);
}

- (NSString *)description
{
    CFStringRef copyDesc = CFCopyDescription((CFTypeRef)self);
    
    return CFBridgingRelease(copyDesc);
}

- (BOOL)allowsWeakReference
{
    return !_CFIsDeallocating(self);
}

- (BOOL)retainWeakReference
{
    return _CFTryRetain(self) != nil;
}

- (CFTypeID)_cfTypeID
{
    return CUIFieldGetTypeID();
}

- (CUICredentialRef)_credentialRef
{
    return (__bridge CUICredentialRef)self;
}

- (NSArray *)fields
{
    return (__bridge NSArray *)CUICredentialGetFields([self _credentialRef]);
}

- init
{
    return [self initWithContext:NULL];
}

- initWithContext:(IUnknownVTbl *)context
{
    CUICredentialRef credentialRef = CUICredentialCreate(kCFAllocatorDefault, context);
    
    self = (__bridge id)credentialRef;
    
    return self;
}

- (CUIField *)firstFieldWithClass:(CUIFieldClass)fieldClass
{
    return (__bridge CUIField *)CUICredentialFindFirstFieldWithClass([self _credentialRef], fieldClass);
}

- (NSDictionary *)attributes
{
    return (__bridge NSDictionary *)CUICredentialGetAttributes([self _credentialRef]);
}

- (void)didBecomeSelected:(BOOL *)pbAutoLogin
{
    Boolean bAutoLogin;
    CUICredentialDidBecomeSelected([self _credentialRef], &bAutoLogin);
    *pbAutoLogin = bAutoLogin;
}

- (void)didBecomeDeselected
{
    CUICredentialDidBecomeDeselected([self _credentialRef]);
}

- (void)willSubmit
{
    CUICredentialWillSubmit([self _credentialRef]);
}

- (void)didSubmit
{
    CUICredentialDidSubmit([self _credentialRef]);
}

- (void)fieldsApplyBlock:(void (^)(CUIFieldRef, BOOL *))block
                    stop:(BOOL *)stop
{
}


- (id)_createGSSItem:(BOOL)addIfNotExisting error:(NSError * __autoreleasing *)error
{
    GSSItemRef item;
    CFErrorRef errorRef = NULL;
    
    item = CUICredentialCreateGSSItem([self _credentialRef],
                                      addIfNotExisting,
                                      &errorRef);
    if (errorRef) {
        if (error)
            *error = CFBridgingRelease(errorRef);
        else
            CFRelease(errorRef);
    }
    
    return CFBridgingRelease(item);
}

- (BOOL)_hasGSSItem
{
    id item = [self _createGSSItem:NO error:NULL];
    
    return !!item;
}

@end
