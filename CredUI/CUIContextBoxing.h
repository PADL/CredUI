//
//  CUIContextBoxing.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CUIIdentityPicker.h>

/*
 * Because GSS contexts are not bridged to the CoreFoundation/ObjC
 * runtimes, the context box is used to allow a caller's context to
 * be replaced (which is necessary to support transferring the context
 * to and from another process, when using ViewBridge).
 *
 * GSSContext in GSSKit.framework implements this protocol.
 *
 * This interface is private.
 *
 * In the future, we should make both GSS and PAM context handles
 * CoreFoundation types, and bridge them to support NSSecureCoding. At
 * that point, we can make this kludge go away.
 */
@protocol CUIContextBoxing <NSObject>

/*
 * Get or set the underlying GSS or PAM context handle.
 */
@property (nonatomic, assign) void *context;

/*
 * Serialize the context handle.
 */
- (NSData *)exportContext;

/*
 * Set the context handle from a serialized context handle.
 */
- (BOOL)importContext:(NSData *)data;

@end

@interface CUIIdentityPicker (CUIContextBoxing)
@property (nonatomic, retain) id <CUIContextBoxing> contextBox;
@end

