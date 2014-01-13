//
//  CUIController.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUIController_h
#define CredUICore_CUIController_h

#include <CredUICore/CUIBase.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __CUIController *CUIControllerRef;

CUI_EXPORT CFTypeID
CUIControllerGetTypeID(void);

CUI_EXPORT CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usage,
                    CUIUsageFlags usageFlags);

CUI_EXPORT CUIUsageScenario
CUIControllerGetUsageScenario(CUIControllerRef controller);

CUI_EXPORT void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef authIdentity);

CUI_EXPORT CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller);

CUI_EXPORT void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError);

CUI_EXPORT CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller);

CUI_EXPORT Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller,
                              CUICredUIContextProperties whichProperties,
                              const CUICredUIContext *context);

CUI_EXPORT const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller);

CUI_EXPORT void
CUIControllerSetGSSContextHandle(CUIControllerRef controller, CFTypeRef ctx);

CUI_EXPORT CFTypeRef
CUIControllerGetGSSContextHandle(CUIControllerRef controller);

CUI_EXPORT void
CUIControllerSetTargetName(CUIControllerRef controller, CFTypeRef target);

CUI_EXPORT CFTypeRef
CUIControllerGetTargetName(CUIControllerRef controller);

CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller,
                                  void (^cb)(CUICredentialRef, Boolean, CFErrorRef));
    
CUI_EXPORT CFStringRef
CUICopyTargetDisplayName(CFTypeRef targetName);

CUI_EXPORT CFStringRef
CUICopyTargetHostName(CFTypeRef targetName);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIController_h */
