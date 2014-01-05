//
//  CUIIdentityTile.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class CUICredential;
@class CUICredentialTileController;

@interface CUICredentialTile : NSView

@property (readonly) CUICredential *credential;
@property (weak, nonatomic) CUICredentialTileController *delegate;
@property (nonatomic, assign) BOOL selected;

@end
