//
//  CUIProxyCredential.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUIProxyCredential_h
#define CredUICore_CUIProxyCredential_h

#include <CredUICore/CUIBase.h>

#ifdef __cplusplus
extern "C" {
#endif
    
CUI_EXPORT CUICredentialRef
CUICredentialCreateProxy(CFAllocatorRef allocator,
                         CFDictionaryRef credAttributes);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIProxyCredential_h */
