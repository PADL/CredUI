//
//  CUIIdentityPicker+Private.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@class CUIIdentityPickerInfo;

@interface CUIIdentityPicker (Private)

- (void)setModalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector;

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker
                  returnCode:(NSInteger)returnCode
                 contextInfo:(void *)contextInfo;

@end
