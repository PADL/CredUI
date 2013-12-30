//
//  CredUICore_Private.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CredUICore_Private_h
#define CredUI_CredUICore_Private_h

#include <CoreFoundation/CFPlugInCOM.h>
#include <CredUICore/CredUICore.h>

#ifdef __cplusplus
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
#endif /* __cplusplus */

CFArrayRef
CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller);

#ifdef __cplusplus
extern "C" {
    extern CUICredentialRef
    CUICredentialCreate(CFAllocatorRef allocator, IUnknown *context);
}
#else
extern CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknownVTbl *context);
#endif

/*
 * Copyright (c) 2011 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Portions Copyright (c) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Create/Modify/Delete/Search GSS items
 *
 * Credentials needs a type, name
 */

#include <GSS/GSS.h>

#ifdef __cplusplus
extern "C" {
#endif

struct GSSItem {
    CFRuntimeBase base;
    CFMutableDictionaryRef keys;
};

typedef struct GSSItem *GSSItemRef;

void
__CUICredentialSetItem(CUICredentialRef cred, GSSItemRef item);
    
GSSItemRef
GSSItemAdd(CFDictionaryRef attributes, CFErrorRef *error)
__attribute__((cf_returns_retained))
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemUpdate(CFDictionaryRef query, CFDictionaryRef attributesToUpdate, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemDelete(CFDictionaryRef query, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemDeleteItem(GSSItemRef item, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/**
 * Will never return a zero length array, GSSItemCopyMatching() will return more then one entry or a NULL pointer.
 */

CFArrayRef
GSSItemCopyMatching(CFDictionaryRef query, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

CFTypeRef
GSSItemGetValue(GSSItemRef item, CFStringRef key)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
    
#ifdef __cplusplus
}
#endif

#endif
