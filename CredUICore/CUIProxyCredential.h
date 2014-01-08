//
//  CUIProxyCredential.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIProxyCredential__
#define __CredUI__CUIProxyCredential__

#ifdef __cplusplus
extern "C" {
#endif
    
CUI_EXPORT CUICredentialRef
CUICredentialCreateProxy(CFAllocatorRef allocator,
                         CFDictionaryRef credAttributes);

#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIProxyCredential__) */
