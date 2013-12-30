//
//  CUIIdentityPicker+Private.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPicker () <NSWindowDelegate>
@property (nonatomic, assign) CUIFlags flags;
@property (nonatomic, assign) CUIControllerRef controller;
@property (nonatomic, retain) CUICredential *selectedCredential;
@property (nonatomic, retain) NSPanel *panel;
@property (nonatomic, retain) NSArrayController *credsController;
@property (nonatomic, retain) NSCollectionView *collectionView;
@end
