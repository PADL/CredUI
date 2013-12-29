//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIIdentityTile
{
    CUICredentialRef _credential;
}

- (void)dealloc
{
    if (_credential)
        CFRelease(_credential);
}

- (id)initWithCredential:(CUICredentialRef)credential
{
    self = [super initWithFrame:NSMakeRect(0, 0, 400, 200)];
    if (self == nil)
        return nil;
    
    _credential = (CUICredentialRef)CFRetain(credential);
    
    NSArray *fields = (__bridge NSArray *)CUICredentialGetFields(credential);
    NSEnumerator *e = [fields objectEnumerator];
    CUIFieldRef field;
    
    NSLog(@"Adding cred %@", _credential);

    while ((field = (__bridge CUIFieldRef)[e nextObject])) {

        NSString *title = (__bridge NSString *)CUIFieldGetTitle(field);
        NSString *defaultValue = (__bridge NSString *)CUIFieldGetDefaultValue(field);
        NSTextField *foo = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 400, 50)];

        NSLog(@"Adding field %@", field);
        
        [foo setStringValue:title];
        [self addSubview:foo];

    }

    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

@end
