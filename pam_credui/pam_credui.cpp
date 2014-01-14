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
#include "CUICFUtilities.h"

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

static int
_PAMCreateAttributesFromHandle(pam_handle_t *pamh, CFDictionaryRef *pAttributes)
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
_PAMConvSelect(const struct pam_conv *conv,
               CFArrayRef creds,
               CFIndex selectedCredentialIndex,
               CUICredentialRef *pSelectedCred)
{
    int rc;
    CFIndex cMessages;
    struct pam_message **messages = NULL;
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
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass(cred, kCUIFieldClassLargeText);
        CFStringRef name = (CFStringRef)CFDictionaryGetValue(attrs, kCUIAttrName);
        CFTypeRef provider = field ? CUIFieldGetDefaultValue(field) : CFDictionaryGetValue(attrs, kCUIAttrCredentialProvider);
        const char *defaultTag = (index == selectedCredentialIndex) ? "*" : "";
        struct pam_message *message;
        CFStringRef displayString;

        message = (struct pam_message *)calloc(1, sizeof(*message));
        if (message == NULL) {
            rc = PAM_BUF_ERR;
            goto cleanup;
        }
        
        if (name)
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld]%s %@ (%@)"),
                                                     index + 1, defaultTag, provider, name);
        else
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld]%s %@"),
                                                     index + 1, defaultTag, provider);
        if (displayString == NULL) {
            rc = PAM_BUF_ERR;
            free(message);
            goto cleanup;
        }
        
        message->msg_style = PAM_TEXT_INFO;
        message->msg = CUICFStringToCString(displayString);
        
        CFRelease(displayString);
        
        messages[index] = message;
    }
    
    struct pam_message *message;

    message = (struct pam_message *)calloc(1, sizeof(*message));
    if (message == NULL) {
        rc = PAM_BUF_ERR;
        goto cleanup;
    }
    message->msg_style = PAM_PROMPT_ECHO_ON;
    message->msg = (char *)"Select a credential";
    
    messages[cMessages] = message;
    
    rc = (conv->conv)((int)cMessages, (const struct pam_message **)messages, &resp, conv->appdata_ptr);
    if (rc != PAM_SUCCESS) {
        free(message);
        goto cleanup;
    }
    
    if (resp && resp->resp) {
        CFIndex index = strtoul(resp->resp, NULL, 10);
        
        if (index && index <= CFArrayGetCount(creds))
            selectedCredentialIndex = index - 1;
    }

    if (selectedCredentialIndex != kCFNotFound) {
        *pSelectedCred = (CUICredentialRef)CFRetain(CFArrayGetValueAtIndex(creds, selectedCredentialIndex));
        rc = PAM_SUCCESS;
    } else {
        rc = PAM_CRED_UNAVAIL;
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
_PAMConvCredential(const struct pam_conv *conv,
                   CUICredentialRef cred)
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
        message->msg = CUICFStringToCString(msg);
        messages[0] = message;
        
        rc = (conv->conv)(1, messages, &resp, conv->appdata_ptr);

        free(message->msg);
        free(message);

        if (rc != PAM_SUCCESS) {
            *stop = true;
            return;
        }
        
        if (resp) {
            if (resp->resp) {
                CFStringRef value;

                value = CFStringCreateWithCString(kCFAllocatorDefault, resp->resp, kCFStringEncodingUTF8);
                if (value) {
                    CUIFieldSetValue(field, value);
                    CFRelease(value);
                }
                
                if (CUIFieldGetClass(field) == kCUIFieldClassPasswordText)
                    memset(resp->resp, 0, strlen(resp->resp));
                free(resp->resp);

                if (value == NULL) {
                    *stop = true;
                    free(resp);
                    return;
                }
            }
            free(resp);
        }
    });
    
    return rc;
}

CUI_EXPORT int
pam_select_credential(pam_handle_t *pamh)
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
    __block CFIndex defaultCredentialIndex = kCFNotFound;

    rc = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    CHECK_STATUS(pamh, "pam_get_item(PAM_CONV)", rc);
    
    controller = CUIControllerCreate(kCFAllocatorDefault, kCUIUsageScenarioLogin, kCUIUsageFlagsDoNotShowUI);
    if (controller == NULL) {
        rc = PAM_SERVICE_ERR;
        goto cleanup;
    }
   
    CUIControllerSetContext(controller, pamh);
 
    creds = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (creds == NULL) {
        rc = PAM_BUF_ERR;
        goto cleanup;
    }
    
    rc = _PAMCreateAttributesFromHandle(pamh, &attributes);
    if (rc == PAM_BUF_ERR)
        goto cleanup;
    else if (attributes)
        CUIControllerSetAttributes(controller, attributes);
    
    CUIControllerEnumerateCredentials(controller, ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
        if (cred) {
            if (isDefault)
                defaultCredentialIndex = CFArrayGetCount(creds);
            CFArrayAppendValue(creds, cred);
        }
    });
    
    do {
        if (selectedCred) {
            CUICredentialDidBecomeDeselected(selectedCred);
            CFRelease(selectedCred);
            selectedCred = NULL;
        }
        
        rc = _PAMConvSelect(conv, creds, defaultCredentialIndex, &selectedCred);
        CHECK_STATUS(pamh, "_PAMConvSelect", rc);
        
        CUICredentialDidBecomeSelected(selectedCred, &autoLogin);
        
        if (!autoLogin) {
            rc = _PAMConvCredential(conv, selectedCred);
            CHECK_STATUS(pamh, "_PAMConvCredential", rc);
        }
        
        CUICredentialWillSubmit(selectedCred);
    } while (!CUICredentialCanSubmit(selectedCred));
    
    CUICredentialDidSubmit(selectedCred);
    
    credAttributes = CUICredentialGetAttributes(selectedCred);
    
    user = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrName));
    if (user) {
        rc = pam_set_item(pamh, PAM_USER, (const void **)user);
        CHECK_STATUS(pamh, "pam_set_item(PAM_USER)", rc);
    }
    
    pass = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrCredentialPassword));
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
