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
{
    NSView *_viewPrototype;
    CUICredential *_credential;
    CUICredentialTileController *_delegate;
    BOOL _selected;
}

@property (nonatomic, weak) IBOutlet NSView *viewPrototype;
@property (nonatomic, readonly) CUICredential *credential;
@property (nonatomic, retain) CUICredentialTileController *delegate;
@property (nonatomic, assign) BOOL selected;

@end
