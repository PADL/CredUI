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
@property(nonatomic, retain) GSSContext *GSSContextHandle;
@property(nonatomic, copy) id targetName;

@property(nonatomic, readonly) CUIFlags flags;
@property(nonatomic, retain, readonly) CUICredential *selectedCredential;
@property(nonatomic, copy) NSError *lastError;

@property(nonatomic, retain, readonly) NSString *targetDisplayName;

@property(nonatomic, retain) NSCollectionView *collectionView;
@property(nonatomic, retain) NSTextField *messageTextField;
@property(nonatomic, retain) NSButton *persistCheckBox;
@property(nonatomic, retain) NSButton *submitButton;

- (NSModalResponse)_runModal:(NSWindow *)window;

- (instancetype)initWithFlags:(CUIFlags)flags
                usageScenario:(CUIUsageScenario)usageScenario
                   attributes:(NSDictionary *)attributes;

- (void)credentialFieldDidChange:(CUICredential *)cred;

- (void)didClickPersist:(id)sender;
- (void)willSubmitCredential:(id)sender;
- (void)didSubmitCredential;

- (void)setCredUIContext:(CUICredUIContext *)uic properties:(CUICredUIContextProperties)props;

@end
