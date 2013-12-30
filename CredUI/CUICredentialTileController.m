//
//  CUICredentialTileController.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTileController
- (void)loadView
{
    [self setView:[[CUICredentialTile alloc] initWithFrame:NSMakeRect(5, 0, 390, 100)]];
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];
    
    if (representedObject) {
        CUICredentialTile *tile = (CUICredentialTile *)self.view;
        [tile setCredential:representedObject];
    }
}

@end
