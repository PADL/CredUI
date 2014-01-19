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

@class CUICredential;

__attribute__((visibility("default")))
@interface CUIIdentityPicker : NSObject
{
    @package
    id _reserved[2];
}

/* Title and message of CredUI window */
@property(nonatomic, copy) NSString *title;
@property(nonatomic, copy) NSString *message;

/* Credentials dictionary */
@property(nonatomic, copy) NSDictionary *attributes;

/* Authentication error to show user, if any (also used with GSS error recovery) */
@property(nonatomic, copy) NSError *authError;

/* Allow user to persist credentials */
@property(nonatomic, assign) BOOL persist;

/* CredUI flags */
@property(nonatomic, readonly) CUIFlags flags;

/* Target name to display to user, one of NSString, NSURL or gss_name_t */
@property(nonatomic, copy) id targetName;

/* GSS or PAM context handle, depending on usage scenario */
@property(nonatomic, assign) const void *context;

/* The credential that was picked */
@property(nonatomic, retain, readonly) CUICredential *selectedCredential;
/* The last error from credential selection */
@property(nonatomic, retain, readonly) NSError *lastError;

/* Create a new IdentityPicker with the specified usage scenario */
- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario;

/*
 * Create a new IdentityPicker with the specified usage scenario and attributes.
 *
 * The attributes will restrict the list of enumerated credentials. For example,
 * specifying kCUIAttrName will only enumerate credentials for a specific user.
 * Attribute constants are in <CredUICore/CUIAttributes.h>.
 */
- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attrs;

/*
 * Create a new IdentityPicker with the specified usage scenario, attributes and flags.
 */
- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attrs
                                flags:(CUIFlags)flags;

/* Run the IdentityPicker as an application-modal panel and return a attribute dictionary. Return NSModalResponseOK or NSModalResponseCancel. */
- (NSInteger)runModal;

/*
 * Run the Identity Picker as a sheet.  The didEndSelector will be invoked after the return value is known but before the sheet is dismissed.
 * The didEndSelector should have the following signature:
 *  - (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo;
 */
- (void)runModalForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector contextInfo:(void *)contextInfo;

@end
