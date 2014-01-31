//
//  CredUIPickerDelegate+InitAcceptLoop.h
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import "CredUIPickerDelegate.h"

@interface CredUIPickerDelegate (InitAcceptLoop)

- (void)doInitAcceptGSSContextWithIdentityPicker:(CUIIdentityPicker *)identityPicker;
- (void)doInitAcceptGSSContext:(GSSContext *)initiatorCtx;
@end
