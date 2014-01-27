//
//  CredUICore_Private.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CredUICore_Private_h
#define CredUICore_CredUICore_Private_h

#include <security/pam_appl.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFPlugInCOM.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFBridgingPriv.h>

#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>
#include <CredUICore/GSSIsPromptingNeeded.h>

#include "CUICFUtilities.h"
#include "CUIGSSUtilities.h"

#undef CUI_EXPORT
#define CUI_EXPORT  __attribute__((visibility("default")))

struct __CUIController {
    CFRuntimeBase _base;
    CFArrayRef _providers;
    CFArrayRef _providerAttributes;
    CUIUsageScenario _usageScenario;
    CUIUsageFlags _usageFlags;
    CUICredUIContext _uiContext;
    CFMutableDictionaryRef _attributes;
    CFErrorRef _authError;
    CFIndex _flags;
    const void *_context; // for use with GSSKit/NegoEx/PAM/etc
    CFTypeRef _targetName;
};

Boolean
_CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller);

CFStringRef
_CUICopyAttrClassForAuthError(CFErrorRef authError);

#ifdef __cplusplus
extern "C" {
#endif
    
void
_CUICleanupPAMAttrData(pam_handle_t *pamh, void *data, int pam_end_status);

Boolean
_CUIIsReturnableCredentialStatus(CFTypeRef status, Boolean *autoLogin);

CUI_EXPORT CFSetRef
_CUIControllerCopyWhitelistedAttributeKeys(CUIControllerRef controller);

CUI_EXPORT CFSetRef
_CUIControllerCopyWhitelistedAttributeClasses(CUIControllerRef controller);

#ifdef __cplusplus
}
#endif

#endif
