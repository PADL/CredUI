//
//  pam_credui.cpp
//  CredUI
//
//  Created by Luke Howard on 14/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifdef DEBUG
#include <dlfcn.h>
#endif

#include <CredUICore/CredUICore_Private.h>

#include "pam_credui.h"

#ifdef DEBUG
#define CHECK_STATUS(pamh, fn, rc)      do { \
        char *_err; \
        _err = (rc == PAM_OPEN_ERR || rc == PAM_SYMBOL_ERR) ? dlerror() : NULL; \
        fprintf(stderr, "%s:%d %s: %s[%d]%s%s\n", \
                __FILE__, __LINE__, (fn), pam_strerror((pamh), (rc)), rc, \
                (rc == PAM_OPEN_ERR || rc == PAM_SYMBOL_ERR) ? " - dlerror: " : "", \
                _err ? _err : "(null)"); \
        if (rc != PAM_SUCCESS) \
            goto cleanup; \
        } while (0)
#else
#define CHECK_STATUS(pamh, fn, rc)      do { \
        if (rc != PAM_SUCCESS) \
            goto cleanup; \
        } while (0)
#endif /* DEBUG */

static char *
_CUICFStringToCString(CFStringRef string)
{
    char *s = NULL;
    
    if (string == NULL)
        return NULL;
    
    CFIndex len = CFStringGetLength(string);
    len = 1 + CFStringGetMaximumSizeForEncoding(len, kCFStringEncodingUTF8);
    s = (char *)malloc(len);
    if (s == NULL)
        return NULL;
    
    if (!CFStringGetCString(string, s, len, kCFStringEncodingUTF8)) {
        free(s);
        s = NULL;
    }
    
    return s;
}

static int
_CUICreateAttributesFromPAMHandle(pam_handle_t *pamh, CFDictionaryRef *pAttributes)
{
    CFMutableDictionaryRef attributes;
    const char *user;
    int rc;
    
    rc = pam_get_item(pamh, PAM_USER, (const void **)&user);
    if (rc != PAM_SUCCESS)
        return rc;
    
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL)
        return PAM_BUF_ERR;
    
    CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, user, kCFStringEncodingUTF8);
    if (name == NULL) {
        CFRelease(attributes);
        return PAM_BUF_ERR;
    }
    
    CFDictionarySetValue(attributes, kCUIAttrNameType, kCUIAttrNameTypePosixName);
    CFDictionarySetValue(attributes, kCUIAttrName, name);
    
    CFRelease(name);
    
    *pAttributes = attributes;
    return PAM_SUCCESS;
}

static void
_PAMConvFreeMessages(struct pam_message **messages, CFIndex count)
{
    for (CFIndex i = 0; i < count; i++) {
        if (messages[i]->msg_style == PAM_TEXT_INFO)
            free(messages[i]->msg);
        free(messages[i]);
    }
    free(messages);
}

static int
_PAMConvSelect(CFArrayRef creds,
               const struct pam_conv *conv,
               CUICredentialRef *pSelectedCred)
{
    int rc;
    CFIndex cMessages;
    struct pam_message **messages = NULL;
    struct pam_message *message;
    struct pam_response *resp = NULL;
    
    *pSelectedCred = NULL;
    
    if (CFArrayGetCount(creds) == 0) {
        return PAM_USER_UNKNOWN;
    } else if (CFArrayGetCount(creds) == 1) {
        *pSelectedCred = (CUICredentialRef)CFRetain(CFArrayGetValueAtIndex(creds, 0));
        return PAM_SUCCESS;
    }
    
    cMessages = CFArrayGetCount(creds) + 1;
    
    messages = (struct pam_message **)calloc(cMessages, sizeof(*messages));
    if (messages == NULL)
        return PAM_BUF_ERR;
    
    for (CFIndex index = 0; index < CFArrayGetCount(creds); index++) {
        CUICredentialRef cred = (CUICredentialRef)CFArrayGetValueAtIndex(creds, index);
        CFDictionaryRef attrs = CUICredentialGetAttributes(cred);
        
        message = (struct pam_message *)calloc(1, sizeof(*message));
        if (message == NULL) {
            rc = PAM_BUF_ERR;
            goto cleanup;
        }
        
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass(cred, kCUIFieldClassLargeText);
        CFStringRef name = (CFStringRef)CFDictionaryGetValue(attrs, kCUIAttrName);
        CFTypeRef provider = field ? CUIFieldGetDefaultValue(field) : CFDictionaryGetValue(attrs, kCUIAttrCredentialProvider);
        CFStringRef displayString;
        
        if (name)
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld] %@ (%@)"), index + 1, provider, name);
        else
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld] %@"), index + 1, provider);
        if (displayString == NULL) {
            rc = PAM_BUF_ERR;
            goto cleanup;
        }
        
        message->msg_style = PAM_TEXT_INFO;
        message->msg = _CUICFStringToCString(displayString);
        
        CFRelease(displayString);
        
        messages[index] = message;
    }
    
    message = (struct pam_message *)calloc(1, sizeof(*message));
    if (message == NULL) {
        rc = PAM_BUF_ERR;
        goto cleanup;
    }
    message->msg_style = PAM_PROMPT_ECHO_ON;
    message->msg = (char *)"Select a credential";
    
    rc = (conv->conv)((int)cMessages, (const struct pam_message **)messages, &resp, conv->appdata_ptr);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
    rc = PAM_CRED_UNAVAIL;
    
    if (resp && resp->resp) {
        CFIndex index = strtoul(resp->resp, NULL, 10);
        
        if (index && index <= CFArrayGetCount(creds)) {
            index--; /* count from zero */
            *pSelectedCred = (CUICredentialRef)CFRetain(CFArrayGetValueAtIndex(creds, index));
            rc = PAM_SUCCESS;
        }
    }
    
cleanup:
    _PAMConvFreeMessages(messages, cMessages);
    if (resp) {
        if (resp->resp)
            free(resp->resp);
        free(resp);
    }
    
    return rc;
}

static int
_PAMConvCredential(CUICredentialRef cred,
                   const struct pam_conv *conv)
{
    CFArrayRef fields = CUICredentialGetFields(cred);
    __block int rc;
    
    if (fields == NULL)
        return PAM_SUCCESS;
    
    CUICredentialFieldsApplyBlock(cred, ^(CUIFieldRef field, Boolean *stop) {
        CFStringRef msg;
        const struct pam_message *messages[1];
        struct pam_message *message;
        int msg_style = 0;
        struct pam_response *resp = NULL;
        
        switch (CUIFieldGetClass(field)) {
            case kCUIFieldClassSmallText: {
                CFTypeRef defaultValue = CUIFieldGetDefaultValue(field);
                
                if (defaultValue && CFGetTypeID(defaultValue) != CFStringGetTypeID())
                    return;
                else
                    msg = (CFStringRef)defaultValue;
                
                msg_style = PAM_TEXT_INFO;
                break;
            }
            case kCUIFieldClassEditText:
                msg_style = PAM_PROMPT_ECHO_ON;
                msg = CUIFieldGetTitle(field);
                break;
            case kCUIFieldClassPasswordText:
                msg_style = PAM_PROMPT_ECHO_OFF;
                msg = CUIFieldGetTitle(field);
                break;
            default:
                return;
        }
        
        message = (struct pam_message *)calloc(1, sizeof(*message));
        if (message == NULL) {
            rc = PAM_BUF_ERR;
            *stop = true;
            return;
        }
        
        message->msg_style = msg_style;
        message->msg = _CUICFStringToCString(msg);
        messages[0] = message;
        
        rc = (conv->conv)(1, messages, &resp, conv->appdata_ptr);
        if (rc != PAM_SUCCESS) {
            *stop = true;
            free(message->msg);
            free(message);
            return;
        }
        
        if (resp) {
            if (resp->resp) {
                CFStringRef value = CFStringCreateWithCString(kCFAllocatorDefault, resp->resp, kCFStringEncodingUTF8);
                
                if (value) {
                    CUIFieldSetValue(field, value);
                    CFRelease(value);
                }
                
                if (message->msg_style == PAM_PROMPT_ECHO_OFF)
                    memset(resp->resp, 0, strlen(resp->resp));
                free(resp->resp);
            }
            free(resp);
        }
        
        free(message->msg);
        free(message);
    });
    
    return rc;
}

CUI_EXPORT int
PAMSelectCredential(pam_handle_t *pamh)
{
    const struct pam_conv *conv;
    int rc;
    CUIControllerRef controller = NULL;
    CFDictionaryRef attributes = NULL;
    CFDictionaryRef credAttributes = NULL;
    CFMutableArrayRef creds = NULL;
    CUICredentialRef selectedCred = NULL;
    char *user = NULL, *pass = NULL;
    Boolean autoLogin = false;
    
    rc = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    CHECK_STATUS(pamh, "pam_get_item(PAM_CONV)", rc);
    
    controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioLogin, kCUIUsageFlagsDoNotShowUI);
    if (controller == NULL) {
        rc = PAM_SERVICE_ERR;
        goto cleanup;
    }
    
    creds = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (creds == NULL) {
        rc = PAM_BUF_ERR;
        goto cleanup;
    }
    
    _CUICreateAttributesFromPAMHandle(pamh, &attributes);
    if (attributes)
        CUIControllerSetAttributes(controller, attributes);
    
    CUIControllerEnumerateCredentials(controller, ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
        if (cred) {
            if (isDefault)
                CFArrayInsertValueAtIndex(creds, 0, cred);
            else
                CFArrayAppendValue(creds, cred);
        }
    });
    
    do {
        if (selectedCred) {
            CUICredentialDidBecomeDeselected(selectedCred);
            CFRelease(selectedCred);
            selectedCred = NULL;
        }
        
        rc = _PAMConvSelect(creds, conv, &selectedCred);
        CHECK_STATUS(pamh, "_PAMConvSelect", rc);
        
        CUICredentialDidBecomeSelected(selectedCred, &autoLogin);
        
        if (!autoLogin) {
            rc = _PAMConvCredential(selectedCred, conv);
            CHECK_STATUS(pamh, "_PAMConvCredential", rc);
        }
        
        CUICredentialWillSubmit(selectedCred);
    } while (!CUICredentialCanSubmit(selectedCred));
    
    CUICredentialDidSubmit(selectedCred);
    
    credAttributes = CUICredentialGetAttributes(selectedCred);
    
    user = _CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrName));
    if (user) {
        rc = pam_set_item(pamh, PAM_USER, (const void **)user);
        CHECK_STATUS(pamh, "pam_set_item(PAM_USER)", rc);
    }
    
    pass = _CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrCredentialPassword));
    if (pass) {
        rc = pam_set_item(pamh, PAM_AUTHTOK, (const void **)pass);
        CHECK_STATUS(pamh, "pam_set_item(PAM_AUTHTOK)", rc);
    }
    
    rc = pam_set_data(pamh, CREDUI_ATTR_DATA, (void *)CFRetain(credAttributes), _CUICleanupPAMAttrData);
    CHECK_STATUS(pamh, "pam_set_data(CREDUI_ATTR_DATA)", rc);
    
cleanup:
    if (controller)
        CFRelease(controller);
    if (attributes)
        CFRelease(attributes);
    if (creds)
        CFRelease(creds);
    if (selectedCred)
        CFRelease(selectedCred);
    if (user)
        free(user);
    if (pass) {
        memset(pass, 0, strlen(pass));
        free(pass);
    }
    
    return rc;
}
