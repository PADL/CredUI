//
//  CUIIdentityTile.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class CUICredential;

@interface CUICredentialTile : NSView

@property (weak, nonatomic) CUICredential *credential;
@property(nonatomic, retain) NSTextField *usernameField;
@property (nonatomic, assign) BOOL selected;

@end
