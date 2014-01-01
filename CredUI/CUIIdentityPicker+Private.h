//
//  CUIIdentityPicker+Private.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPicker () <NSWindowDelegate>

@property(nonatomic, assign) CUIFlags flags;
@property(nonatomic, assign) CUIControllerRef controller;
@property(nonatomic, retain) CUICredential *selectedCredential;
@property(nonatomic, assign) CUICredUIContext *credUIContext;
@property(nonatomic, retain) NSPanel *panel;
@property(nonatomic, retain) NSArrayController *credsController;
@property(nonatomic, retain) NSCollectionView *collectionView;
@property(nonatomic, retain) NSTextField *messageTextField;
@property(nonatomic, readonly) NSString *targetDisplayName;

- (void)willSubmitCredential:(id)sender;
- (void)didSubmitCredential;

@end
