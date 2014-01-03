//
//  CUIIdentityPickerInternalWindowController.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInternal : NSWindowController  <NSWindowDelegate>

@property(nonatomic, copy) NSString *title;
@property(nonatomic, copy) NSString *message;
@property(nonatomic, copy) NSDictionary *attributes;
@property(nonatomic, copy) NSError *authError;
@property(nonatomic, assign) BOOL persist;
@property(nonatomic, readonly) CUIFlags flags;
@property(nonatomic, retain) GSSContext *GSSContextHandle;
@property(nonatomic, copy) id targetName;

@property(nonatomic, assign) CUIControllerRef controller;
@property(nonatomic, assign) CUICredUIContext *credUIContext;

@property(nonatomic, retain, readonly) CUICredential *selectedCredential;
@property(nonatomic, retain, readonly) NSString *targetDisplayName;
@property(nonatomic, retain, readonly) NSString *targetHostName;

@property(nonatomic, retain) NSArrayController *credsController;
@property(nonatomic, retain) NSCollectionView *collectionView;
@property(nonatomic, retain) NSTextField *messageTextField;
@property(nonatomic, retain) NSButton *submitButton;
@property(assign) BOOL autoLogin;

- (NSModalResponse)_runModal:(NSWindow *)window;

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes;

- (void)didClickPersist:(id)sender;
- (void)credentialFieldDidChange:(id)sender;
- (void)willSubmitCredential:(id)sender;
- (void)didSubmitCredential;

@end
