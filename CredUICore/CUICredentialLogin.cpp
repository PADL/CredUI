//
//  CUICredentialLogin.cpp
//  CredUI
//
//  Created by Luke Howard on 13/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

CUI_EXPORT void
_CUICleanupPAMAttrData(pam_handle_t *pamh, void *data, int pam_end_status)
{
    CFRelease((CFTypeRef)data);
}

static int _CUIPAMConversation(int num_msg,
                               const struct pam_message **msgv,
                               struct pam_response **respv,
                               void *appdata_ptr)
{
    if (respv)
        *respv = NULL;
    return PAM_CONV_ERR;
}

CUI_EXPORT Boolean
CUICredentialAuthenticateForLoginScenario(CUICredentialRef credential, CFStringRef pamService)
{
    CF_OBJC_FUNCDISPATCHV(CUICredentialGetTypeID(), Boolean, credential, "authenticateForLoginScenario:", pamService);

    int rc = PAM_CRED_UNAVAIL;
    CFDictionaryRef attrs;
    char *service = NULL;
    char *user = NULL;
    char *pass = NULL;
    pam_handle_t *pamh = NULL;
    struct pam_conv conv = { .conv = _CUIPAMConversation, .appdata_ptr = credential };

    attrs = CUICredentialGetAttributes(credential);
    if (attrs == NULL)
        goto cleanup;

    service = CUICFStringToCString(pamService);
    
    user = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(attrs, kCUIAttrName));
    if (user == NULL)
        goto cleanup;
    
    rc = pam_start(service ? service : "login", user, &conv, &pamh);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
    pass = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(attrs, kCUIAttrCredentialPassword));
    if (pass) {
        rc = pam_set_item(pamh, PAM_AUTHTOK, (void *)pass);
        if (rc != PAM_SUCCESS)
            goto cleanup;
    }
 
    rc = pam_set_data(pamh, CREDUI_ATTR_DATA, (void *)CFRetain(attrs), _CUICleanupPAMAttrData);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
    rc = pam_authenticate(pamh, 0);
    if (rc != PAM_SUCCESS)
        goto cleanup;

    rc = pam_acct_mgmt(pamh, 0);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
cleanup:
    if (service)
        free(service);
    if (user)
        free(user);
    if (pass) {
        memset(pass, 0, strlen(pass));
        free(pass);
    }
    if (pamh)
        pam_end(pamh, rc);
    
    return (rc == PAM_SUCCESS);
}
