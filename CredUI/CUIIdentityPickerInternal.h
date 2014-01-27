//
//  CUIIdentityPickerInternal.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

__attribute__((visibility("default")))
@interface CUIIdentityPickerInternal : NSWindowController  <NSWindowDelegate>
{
    NSString *_title;
    NSString *_message;
    NSDictionary *_attributes;
    NSError *_authError;
    BOOL _persist;
    const void *_context;
    id _targetName;

    CUIFlags _flags;
    CUIUsageScenario _usageScenario;
    CUIUsageFlags _usageFlags;

    NSError *_lastError;

    NSCollectionView *_collectionView;
    NSTextField *_titleTextField;
    NSTextField *_messageTextField;
    NSButton *_persistCheckBox;
    NSButton *_submitButton;
    NSButton *_cancelButton;

    CUIControllerRef _controllerRef;
    CUICredUIContext *_credUIContext;
    NSSet *_whitelistedAttributeKeys;

    NSArrayController *_credsController;
    
    BOOL _runningModal;
    BOOL _autoLogin;
}

@property(nonatomic, copy) NSString *title;
@property(nonatomic, copy) NSString *message;
@property(nonatomic, copy) NSDictionary *attributes;
@property(nonatomic, copy) NSError *authError;
@property(nonatomic, assign) BOOL persist;
@property(nonatomic, assign) const void *context;
@property(nonatomic, copy) id targetName;

@property(nonatomic, readonly) CUIUsageScenario usageScenario;
@property(nonatomic, readonly) CUIFlags flags;
@property(nonatomic, readonly) CUIUsageFlags usageFlags;

@property(nonatomic, retain, readonly) CUICredential *selectedCredential;
@property(nonatomic, copy) NSError *lastError;

@property(nonatomic, retain, readonly) NSString *targetDisplayName;

@property(assign) IBOutlet NSCollectionView *collectionView;
@property(assign) IBOutlet NSTextField *titleTextField;
@property(assign) IBOutlet NSTextField *messageTextField;
@property(assign) IBOutlet NSButton *persistCheckBox;
@property(assign) IBOutlet NSButton *submitButton;
@property(assign) IBOutlet NSButton *cancelButton;

@property(nonatomic, readonly) CUIControllerRef controllerRef;
@property(nonatomic, readonly) NSSet *whitelistedAttributeKeys;

@property(assign) IBOutlet NSArrayController *credsController;

- (IBAction)didClickPersist:(id)sender;
- (IBAction)didClickOK:(id)sender;
- (IBAction)didClickCancel:(id)sender;
 
- (void)willCancelCredential:(id)sender;
- (void)willSubmitCredential:(id)sender;
- (void)didSubmitCredential;

- (void)setCredUIContext:(CUICredUIContext *)uic properties:(CUICredUIContextProperties)props;

- (NSInteger)runModal;
- (void)runModalForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;

- (void)startCredentialEnumeration;
- (void)endCredentialEnumeration:(NSModalResponse)modalResponse;

- (BOOL)configureForUsageScenario:(CUIUsageScenario)usageScenario
                            flags:(CUIFlags)usageFlags;
- (BOOL)isConfigured;

@end
