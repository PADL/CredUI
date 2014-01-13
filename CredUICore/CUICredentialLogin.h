//
//  CUICredentialLogin.h
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUICredentialLogin_h
#define CredUICore_CUICredentialLogin_h

#ifdef __cplusplus
extern "C" {
#endif
    
CUI_EXPORT Boolean
CUICredentialAuthenticateForLoginScenario(CUICredentialRef credential, CFStringRef pamService);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUICredentialLogin_h */