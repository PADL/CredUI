//
//  CUIChooseIdentityPanel.h
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <AppKit/NSPanel.h>

/* CredUI flags, similar to Windows CredUI API */
typedef NS_OPTIONS(NSUInteger, CUIFlags) {
    CUIIncorrectPassword            = 0x00001,
    CUIDoNotPersist                 = 0x00002,
    CUIRequestAdministrator         = 0x00004,
    CUIExcludeCertificates          = 0x00008,
    CUIRequireCertificate           = 0x00010,
    CUIShowSaveCheckBox             = 0x00040,
    CUIAlwaysShowUI                 = 0x00080,
    CUIRequireSmartcard             = 0x00100,
    CUIPasswordOnlyOK               = 0x00200,
    CUIValidateUsername             = 0x00400,
    CUICompleteUsername             = 0x00800,
    CUIPersist                      = 0x01000,
    CUIFlagsServerCredential        = 0x04000,
    CUIFlagsExpectConfirmation      = 0x20000,
    CUIFlagsGenericCredentials      = 0x40000,
    CUIFlagsUsernameTargetCredentials = 0x80000,
    CUIFlagsKeepUsername            = 0x100000
};

@interface CUIChooseIdentityPanel : NSPanel

+ (CUIChooseIdentityPanel *)sharedChooseIdentityPanel;

- (NSInteger)runModal;

- (void)beginSheet:(NSWindow *)sheet
    modalForWindow:(NSWindow *)docWindow
       didEndBlock: (void (^)(NSInteger returnCode))block;


/* Credentials dictionary, can be passed into GSSItem */
@property(nonatomic, copy) NSDictionary *authIdentity;
/* Authentication error to show user */
@property(nonatomic, assign) OSStatus authError;
/* Allow user to persist credentials */
@property(nonatomic, assign) BOOL save;
/* CredUI flags */
@property(nonatomic, assign) CUIFlags flags;

/* GSS-API context handle for NegoEx exchange */
@property(nonatomic, retain) id gssContextHandle;

/* Target name to display to user */
@property(nonatomic, copy) NSString *targetName;

/* Convenience accessors into authIdentity */

/* GSS-API credential handle, convenience for authIdentity dict */
@property(nonatomic, readonly) id gssCredHandle;
/* SASL mechanism name (kGSSAttrClass) */
@property(nonatomic, copy) NSString *mechanismName;
/* GSS mechanism OID (kGSSAttrClass) */
@property(nonatomic, copy) NSString *mechanismOid;
/* Username (kGSSAttrName/kGSSAttrNameTypeGSSUsername) */
@property(nonatomic, copy) NSString *userName;

@end