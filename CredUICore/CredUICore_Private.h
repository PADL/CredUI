//
//  CredUICore_Private.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CredUICore_Private_h
#define CredUICore_CredUICore_Private_h

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreFoundation/CFRuntime.h>

#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/GSSIsPromptingNeeded.h>
#include "CFBridgeHelper.h"

#undef CUI_EXPORT
#define CUI_EXPORT  __attribute__((visibility("default")))

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
_CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller);

CFStringRef
_CUICopyAttrClassForAuthError(CFErrorRef authError);

#endif
