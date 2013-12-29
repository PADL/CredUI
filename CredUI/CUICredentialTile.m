//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTile
{
    id _credential;
}

- (id)credential
{
    return _credential;
}

- (CUICredentialRef)credentialRef
{
    return (__bridge CUICredentialRef)self.credential;
}

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        self.textField = [[NSTextField alloc] initWithFrame:frameRect];
        [self addSubview:self.textField];
    }
    return self;
}

- (void)setCredential:(id)credential
{
    _credential = credential;
    
    CUIFieldRef field = CUICredentialFindFirstFieldWithClass([self credentialRef], kCUIFieldClassLargeText);
    self.textField.stringValue = (__bridge NSString *)CUIFieldGetTitle(field);
}

@end
