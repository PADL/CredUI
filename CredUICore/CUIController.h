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

/*
 * CUIController type ID
 */
CUI_EXPORT CFTypeID
CUIControllerGetTypeID(void);

/*
 * Creates a new CredUI controller. Controllers are used to enumerate credentials
 * vended by credential providers. The usage scenario refers to whether the caller
 * is performing a local login (kCUIUsageScenarioLogin) or a network authentication
 * (kCUIUsageScenarioNetwork).
 *
 * Credential providers are dynamically loaded from /Library/CredentialProviders
 * and are COM plugins that implement CUICredentialContext.
 */
CUI_EXPORT CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usage,
                    CUIUsageFlags usageFlags);

/*
 * Returns the usage scenario for a controller.
 */
CUI_EXPORT CUIUsageScenario
CUIControllerGetUsageScenario(CUIControllerRef controller);

/*
 * Returns the usage flags for a controller.
 */
CUI_EXPORT CUIUsageFlags
CUIControllerGetUsageFlags(CUIControllerRef controller);

/*
 * Sets the attributes associated with a controller. These are passed to each credential
 * provider and are used to filter the results, e.g. to only return credentials that
 * match a particular username (kCUIAttrName) or class (kCUIAttrClass). The attributes
 * are copied.
 */
CUI_EXPORT void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef attributes);

/*
 * Returns the attributes previously set with CUIControllerSetAttributes.
 */
CUI_EXPORT CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller);

/*
 * Sets the authentication error associated with a controller. This is passed to each
 * credential provider and is used to tailor the UI to a particular authentication
 * error. Particular types of error may have special handling by CredUICore.
 */
CUI_EXPORT void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError);

/*
 * Returns the authentication error previously set with CUIControllerSetAuthError.
 */
CUI_EXPORT CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller);

/*
 * Sets the UI context associated with a controller. CredUICore does not use this
 * information directly as it does not display an UI itself, however the information
 * is passed to credential providers.
 */
CUI_EXPORT Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller,
                              CUICredUIContextProperties whichProperties,
                              const CUICredUIContext *context);

/*
 * Returns the UI context previously set with CUIControllerSetCredUIContext.
 */
CUI_EXPORT const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller);

/*
 * Associates an opaque context value with the controller which is made available to
 * credential providers. By convention this is a pam_handle_t for kCUIUsageScenarioLogin
 * and a gss_ctx_id_t for kCUIUsageScenarioNetwork. The context must be valid for the
 * lifetime of the controller.
 */
CUI_EXPORT void
CUIControllerSetContext(CUIControllerRef controller, const void *ctx);

/*
 * Returns the opaque context previously set with CUIControllerSetContext.
 */
CUI_EXPORT const void *
CUIControllerGetContext(CUIControllerRef controller);

/*
 * Associate the target of the authentication request. This is the PAM service for
 * kCUIUsageScenarioLogin and a server name (gss_name_t, CFURL or CFString are all
 * supported) for kCUIUsageScenarioNetwork.
 */
CUI_EXPORT void
CUIControllerSetTargetName(CUIControllerRef controller, CFTypeRef target);

/*
 * Returns the target name previously set with CUIControllerSetTargetName.
 */
CUI_EXPORT CFTypeRef
CUIControllerGetTargetName(CUIControllerRef controller);

/*
 * Enumerates credentials for each credential provider loaded by the controller.
 */
CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller,
                                  void (^cb)(CUICredentialRef, Boolean, CFErrorRef));

/*
 * Turns a target name into a string.
 */    
CUI_EXPORT CFStringRef
CUICopyTargetDisplayName(CFTypeRef targetName);

/*
 * Extracts the host name from a target name.
 */
CUI_EXPORT CFStringRef
CUICopyTargetHostName(CFTypeRef targetName);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIController_h */
