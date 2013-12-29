//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUICredential__
#define __CredUI__CUICredential__

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct __CUICredential *CUICredentialRef;

extern CFTypeID
CUICredentialGetTypeID(void);    
    
extern CFArrayRef
CUICredentialGetFields(CUICredentialRef cred);
    
extern CFDictionaryRef
CUICredentialGetAuthIdentity(CUICredentialRef cred);
        
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUICredential__) */