//
//  CUIIdentityPicker+CredentialSubmission.m
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//


@implementation CUIIdentityPicker (CredentialSubmission)
- (BOOL)_canReturnWithCredential:(CUICredential *)cred
{
    NSDictionary *attrs = cred.attributes;
    id status = [attrs objectForKey:(__bridge id)kCUICredentialStatus];
    
    if ([status isEqual:(__bridge id)kCUICredentialReturnCredentialFinished] ||
        [status isEqual:(__bridge id)kCUICredentialReturnNoCredentialFinished])
        return YES;
    else
        return NO;
}

- (void)_selectCredential:(CUICredential *)cred
{
    NSArray *selected = @[ cred ];
    
    [self.credsController setSelectedObjects:selected];
    [self.panel close];
}
@end
