//
//  CUIIdentityPicker.h
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <Foundation/NSObject.h>

@class NSString;
@class NSDictionary;
@class GSSContext;
@class GSSItem;
@class CUICredential;

/* CredUI flags, similar to Windows CredUI API */
typedef NS_OPTIONS(NSUInteger, CUIFlags) {
    CUIIncorrectPassword                = 0x00001,
    CUIDoNotPersist                     = 0x00002,
    CUIRequestAdministrator             = 0x00004,
    CUIExcludeCertificates              = 0x00008,
    CUIRequireCertificate               = 0x00010,
    CUIShowSaveCheckBox                 = 0x00040,
    CUIAlwaysShowUI                     = 0x00080,
    CUIRequireSmartcard                 = 0x00100,
    CUIPasswordOnlyOK                   = 0x00200,
    CUIValidateUsername                 = 0x00400,
    CUICompleteUsername                 = 0x00800,
    CUIPersist                          = 0x01000,
    CUIFlagsServerCredential            = 0x04000,
    CUIFlagsExpectConfirmation          = 0x20000,
    CUIFlagsGenericCredentials          = 0x40000,
    CUIFlagsUsernameTargetCredentials   = 0x80000,
    CUIFlagsKeepUsername                = 0x100000
};

@interface CUIIdentityPicker : NSObject
@property(nonatomic, copy) NSString *title;
@property(nonatomic, copy) NSString *message;

/* Credentials dictionary */
@property(nonatomic, copy) NSDictionary *attributes;

/* Authentication error to show user, if any */
@property(nonatomic, copy) NSError *authError;

/* Allow user to persist credentials */
@property(nonatomic, assign) BOOL saveToKeychain;

/* CredUI flags */
@property(nonatomic, readonly) CUIFlags flags;

/* GSS-API context handle for NegoEx exchange */
@property(nonatomic, retain) GSSContext *GSSContextHandle;

/* Target name to display to user */
/* This can be a GSSName or a NSString */
@property(nonatomic, copy) id targetName;

@property(nonatomic, readonly, retain) CUICredential *selectedCredential;
@property(nonatomic, readonly, copy) NSDictionary *selectedCredentialAttributes;

/* Creates a new/find an existing GSSItem for selected credential */
- (GSSItem *)selectedGSSItem:(NSError * __autoreleasing *)error;

- initWithFlags:(CUIFlags)flags;

- initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attrs;

/* Run the IdentityPicker as an application-modal panel and return a attribute dictionary. Return NSOKButton or NSCancelButton. */
- (NSInteger)runModal;

/* Run the Identity Picker as a sheet.  The didEndSelector will be invoked after the return value is known but before the sheet is dismissed.
   The didEndSelector should have the following signature:
    - (void)identityPickerDidEnd:(id)attributeDictOrGSSItem returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;
*/
- (void)runModalForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;

@end
