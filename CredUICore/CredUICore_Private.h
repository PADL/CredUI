//
//  CredUICore_Private.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CredUICore_Private_h
#define CredUI_CredUICore_Private_h

#include "CredUICore.h"

static inline void
__CUISetter(CFTypeRef &dst, CFTypeRef src)
{
    if (dst != src) {
        if (dst) {
            CFRelease(dst);
            dst = NULL;
        }
        if (src) {
            dst = CFRetain(src);
        }
    }
}

CFArrayRef
CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller);

extern CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknown *context);

#endif