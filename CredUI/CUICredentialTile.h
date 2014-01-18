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
    __weak CUICredentialTileController *_delegate;
    BOOL _selected;
}

@property (nonatomic, retain) IBOutlet NSView *viewPrototype;
@property (readonly) CUICredential *credential;
@property (weak, nonatomic) CUICredentialTileController *delegate;
@property (nonatomic, assign) BOOL selected;

@end
