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

    __weak NSCollectionView *_collectionView;
    __weak NSTextField *_titleTextField;
    __weak NSTextField *_messageTextField;
    __weak NSButton *_persistCheckBox;
    __weak NSButton *_submitButton;
    __weak NSButton *_cancelButton;

    CUIControllerRef _controllerRef;
    CUICredUIContext *_credUIContext;

    __weak NSArrayController *_credsController;
    
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

@property(nonatomic, weak) IBOutlet NSCollectionView *collectionView;
@property(nonatomic, weak) IBOutlet NSTextField *titleTextField;
@property(nonatomic, weak) IBOutlet NSTextField *messageTextField;
@property(nonatomic, weak) IBOutlet NSButton *persistCheckBox;
@property(nonatomic, weak) IBOutlet NSButton *submitButton;
@property(nonatomic, weak) IBOutlet NSButton *cancelButton;

@property(nonatomic, readonly) CUIControllerRef controllerRef;

@property(nonatomic, weak) IBOutlet NSArrayController *credsController;

- (IBAction)didClickPersist:(id)sender;
- (IBAction)didClickOK:(id)sender;
- (IBAction)didClickCancel:(id)sender;
 
- (void)willCancelCredential:(id)sender;
- (void)willSubmitCredential:(id)sender;
- (void)didSubmitCredential;

- (void)setCredUIContext:(CUICredUIContext *)uic properties:(CUICredUIContextProperties)props;

- (NSInteger)runModal;

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow completionHandler:(void (^)(NSModalResponse returnCode))handler;

- (void)startCredentialEnumeration:(NSWindow *)parentWindow;
- (void)endCredentialEnumeration:(NSModalResponse)modalResponse;

- (BOOL)configureForUsageScenario:(CUIUsageScenario)usageScenario
                            flags:(CUIFlags)usageFlags;
- (BOOL)isConfigured;

@end
