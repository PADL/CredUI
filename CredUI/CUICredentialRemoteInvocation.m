//
//  CUICredentialRemoteInvocation.m
//  CredUI
//
//  Created by Luke Howard on 31/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUICredentialRemoteInvocation ()
@property(nonatomic, retain) NSUUID *invocationID;
@end

@implementation CUICredentialRemoteInvocation

@synthesize credentialID = _credentialID;
@synthesize invocationID = _invocationID;
@synthesize selector = _selector;
@synthesize error = _error;

- init
{
    if ((self = [super init]) == nil)
        return nil;
    
    self.invocationID = [NSUUID UUID];
    
    return self;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.invocationID forKey:@"invocationID"];
    [coder encodeObject:self.credentialID forKey:@"credentialID"];
    [coder encodeObject:self.selector forKey:@"selector"];
    [coder encodeObject:self.error forKey:@"error"];
}

- (id)initWithCoder:(NSCoder *)coder
{
    self.invocationID = [coder decodeObjectOfClass:[NSUUID class] forKey:@"invocationID"];
    self.credentialID = [coder decodeObjectOfClass:[NSUUID class] forKey:@"credentialID"];
    self.selector = [coder decodeObjectOfClass:[NSString class] forKey:@"selector"];
    self.error = [coder decodeObjectOfClass:[NSError class] forKey:@"error"];
    
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<CUICredentialRemoteInvocation %p{credentialID = \"%@\", invocationID = \"%@\", selector = \"%@\"}>",
            self, self.credentialID.UUIDString, self.invocationID.UUIDString, self.selector];
}

@end
