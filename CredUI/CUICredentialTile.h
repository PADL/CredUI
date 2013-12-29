//
//  CUIIdentityTile.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUICredentialTile : NSView

@property (weak, nonatomic) id credential;
@property (nonatomic, retain) NSTextField *textField;

@end
