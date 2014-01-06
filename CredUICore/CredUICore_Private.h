//
//  CredUICore_Private.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CredUICore_Private_h
#define CredUI_CredUICore_Private_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreFoundation/CFRuntime.h>

#include <GSS/GSS.h>

#define CUI_EXPORT  __attribute__((visibility("default")))

#include <CredUICore/CredUICore.h>
#include "CFBridgeHelper.h"

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

struct __CUIController {
    CFRuntimeBase _base;
    CFArrayRef _factories;
    CFArrayRef _providers;
    CUIUsageScenario _usage;
    CUIUsageFlags _usageFlags;
    CUICredUIContext _uiContext;
    CFMutableDictionaryRef _attributes;
    CFErrorRef _authError;
    CFIndex _flags;
    CFTypeRef _gssContextHandle; // for use with GSSKit/NegoEx
    CFTypeRef _targetName;
};

Boolean
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

#endif
