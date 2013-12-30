//
//  CUIIdentityPicker+CredentialSubmission.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPicker (CredentialSubmission)
- (BOOL)_canReturnWithCredential:(CUICredential *)cred;
- (void)_selectCredential:(CUICredential *)cred;
@end
