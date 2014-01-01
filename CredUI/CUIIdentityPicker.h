//
//  CUIIdentityPicker.h
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <Foundation/NSObject.h>
#import <CredUI/CUIPromptForCredentials.h>

@class NSString;
@class NSDictionary;
@class NSWindow;

@class GSSContext;
@class GSSItem;
@class CUICredential;

__attribute__((visibility("default")))
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

@property(nonatomic, retain, readonly) CUICredential *selectedCredential;

- (instancetype)initWithFlags:(CUIFlags)flags;

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attrs;

/* Run the IdentityPicker as an application-modal panel and return a attribute dictionary. Return NSOKButton or NSCancelButton. */
- (NSInteger)runModal;

/* Run the Identity Picker as a sheet.  The didEndSelector will be invoked after the return value is known but before the sheet is dismissed.
   The didEndSelector should have the following signature:
    - (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;
*/
- (void)runModalForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;

@end
