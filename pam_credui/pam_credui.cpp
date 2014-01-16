//
//  pam_credui.cpp
//  CredUI
//
//  Created by Luke Howard on 14/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CredUICore/CredUICore_Private.h>
#include "CUICFUtilities.h"

#include "pam_credui.h"

static int
_PAMCreateAttributesFromHandle(pam_handle_t *pamh, CFDictionaryRef *pAttributes)
{
    CFMutableDictionaryRef attributes = NULL;
    const char *user;
    int rc;

    rc = pam_get_item(pamh, PAM_USER, (const void **)&user);
    if (rc != PAM_SUCCESS)
        return rc;

    CFStringRef name = CFStringCreateWithCString(kCFAllocatorDefault, user, kCFStringEncodingUTF8);
    if (name == NULL) {
        CFRelease(attributes);
        return PAM_BUF_ERR;
    }

    /* In case module returned PAM_TRY_AGAIN */
    rc = pam_get_data(pamh, CREDUI_ATTR_DATA, (const void **)&attributes);
    if (rc == PAM_SUCCESS && attributes) {
        CFStringRef assertedName = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);
        
        if (assertedName && CFEqual(assertedName, name)) {
            *pAttributes = (CFDictionaryRef)CFRetain(attributes);
            CFRelease(name);
            return PAM_SUCCESS;
        }
    }
    
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL)
        return PAM_BUF_ERR;
    
    CFDictionarySetValue(attributes, kCUIAttrNameType, kCUIAttrNameTypePosixName);
    CFDictionarySetValue(attributes, kCUIAttrName, name);
    
    /* we don't get AUTHTOK because it might prompt */
    
    CFRelease(name);
    
    *pAttributes = attributes;
    return PAM_SUCCESS;
}

static void
_PAMConvFreeResponses(CFIndex count, struct pam_response *resp)
{
    CFIndex index;

    if (resp == NULL)
        return;

    for (index = 0; index < count; index++) {
        struct pam_response *r = &resp[index];

        if (r->resp) {
            memset(r->resp, 0, strlen(r->resp));
            free(r->resp);
        }
    }
    free(resp);
}

static void
_PAMConvFreeMessages(CFIndex count, struct pam_message **messages)
{
    for (CFIndex i = 0; i < count; i++) {
        struct pam_message *message = messages[i];

        if (message->msg)
            free(message->msg);
        free(message);
    }

    free(messages);
}

static int
_PAMConvSelect(pam_handle_t *pamh,
               int flags,
               CFArrayRef creds,
               CFIndex selectedCredentialIndex,
               CUICredentialRef *pSelectedCred)
{
    int rc;
    const struct pam_conv *conv;
    CFIndex iCred, cMessages;
    struct pam_message **messages = NULL;
    struct pam_response *resp = NULL;
    
    *pSelectedCred = NULL;
    
    if (CFArrayGetCount(creds) == 0) {
        return PAM_USER_UNKNOWN;
    } else if (CFArrayGetCount(creds) == 1) {
        *pSelectedCred = (CUICredentialRef)CFRetain(CFArrayGetValueAtIndex(creds, 0));
        return PAM_SUCCESS;
    } else if (flags & PAM_SILENT) {
        return PAM_CRED_UNAVAIL;
    }
    
    rc = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    if (rc != PAM_SUCCESS)
        return rc;
    else if (conv == NULL)
        return PAM_CRED_UNAVAIL;

    cMessages = CFArrayGetCount(creds) + 1;
    
    messages = (struct pam_message **)calloc(cMessages, sizeof(*messages));
    if (messages == NULL)
        return PAM_BUF_ERR;
    
    for (iCred = 0; iCred < CFArrayGetCount(creds); iCred++) {
        CUICredentialRef cred = (CUICredentialRef)CFArrayGetValueAtIndex(creds, iCred);
        CFDictionaryRef attrs = CUICredentialGetAttributes(cred);
        CUIFieldRef field = CUICredentialFindFirstFieldWithClass(cred, kCUIFieldClassLargeText);
        CFStringRef name = (CFStringRef)CFDictionaryGetValue(attrs, kCUIAttrName);
        CFTypeRef provider = field ? CUIFieldGetDefaultValue(field) : CFDictionaryGetValue(attrs, kCUIAttrCredentialProvider);
        const char *defaultTag = (iCred == selectedCredentialIndex) ? "*" : "";
        struct pam_message *message;
        CFStringRef displayString;

        message = (struct pam_message *)calloc(1, sizeof(*message));
        if (message == NULL) {
            rc = PAM_BUF_ERR;
            goto cleanup;
        }
        
        if (name)
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld]%s %@ (%@)"),
                                                     iCred + 1, defaultTag, provider, name);
        else
            displayString = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("[%ld]%s %@"),
                                                     iCred + 1, defaultTag, provider);
        if (displayString == NULL) {
            rc = PAM_BUF_ERR;
            free(message);
            goto cleanup;
        }
        
        message->msg_style = PAM_TEXT_INFO;
        message->msg = CUICFStringToCString(displayString);
        
        CFRelease(displayString);
        
        messages[iCred] = message;
    }
    
    struct pam_message *message;

    message = (struct pam_message *)calloc(1, sizeof(*message));
    if (message == NULL) {
        rc = PAM_BUF_ERR;
        goto cleanup;
    }
    message->msg_style = PAM_PROMPT_ECHO_ON;
    message->msg = strdup("Select a credential: ");
    
    messages[iCred] = message;
    
    rc = (conv->conv)((int)cMessages, (const struct pam_message **)messages, &resp, conv->appdata_ptr);
    if (rc != PAM_SUCCESS) {
        free(message);
        goto cleanup;
    }
    
    if (resp && resp[iCred].resp) {
        CFIndex iSelected = strtoul(resp[iCred].resp, NULL, 10); /* index 1..N */
        
        if (iSelected && iSelected <= CFArrayGetCount(creds))
            selectedCredentialIndex = iSelected - 1;
    }

    if (selectedCredentialIndex != kCFNotFound) {
        *pSelectedCred = (CUICredentialRef)CFRetain(CFArrayGetValueAtIndex(creds, selectedCredentialIndex));
        rc = PAM_SUCCESS;
    } else {
        rc = PAM_CRED_UNAVAIL;
    }
 
cleanup:
    _PAMConvFreeMessages(cMessages, messages);
    _PAMConvFreeResponses(cMessages, resp);
    
    return rc;
}

static Boolean
_PAMMapCUIField(CUIFieldRef field,
                int *pMessageStyle,
                CFStringRef *messageText)
{
    int messageStyle = 0;

    switch (CUIFieldGetClass(field)) {
        case kCUIFieldClassSmallText: {
            CFTypeRef defaultValue = CUIFieldGetDefaultValue(field);
            
            if (defaultValue && CFGetTypeID(defaultValue) != CFStringGetTypeID())
                return false;
            else if (messageText)
                *messageText = (CFStringRef)CFRetain(defaultValue);
            
            messageStyle = PAM_TEXT_INFO;
            break;
        }
        case kCUIFieldClassEditText:
        case kCUIFieldClassPasswordText:
            messageStyle = CUIFieldGetClass(field) == kCUIFieldClassPasswordText ? PAM_PROMPT_ECHO_ON : PAM_PROMPT_ECHO_OFF;
            if (messageText)
                *messageText = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@: "), CUIFieldGetTitle(field));
            break;
        default:
            break;
    }

    if (pMessageStyle)
        *pMessageStyle = messageStyle;

    return !!messageStyle;
}

static int
_PAMConvCredential(pam_handle_t *pamh,
                   int flags,
                   CUICredentialRef cred)
{
    const struct pam_conv *conv = NULL;
    CFArrayRef fields = CUICredentialGetFields(cred);
    CFIndex cFields, cMessages;
    struct pam_message **messages = NULL;
    struct pam_response *resp = NULL;
    CFIndex index;
    __block int rc;
    
    if (fields == NULL)
        return PAM_SUCCESS;
    else if (flags & PAM_SILENT)
        return PAM_AUTHINFO_UNAVAIL;

    rc = pam_get_item(pamh, PAM_CONV, (const void **)&conv);
    if (rc != PAM_SUCCESS)
        return rc;
    else if (conv == NULL)
        return PAM_AUTHINFO_UNAVAIL;

    cFields = CFArrayGetCount(fields);

    messages = (struct pam_message **)calloc(cFields, sizeof(*messages));
    if (messages == NULL)
        return PAM_BUF_ERR;

    for (index = 0, cMessages = 0; index < CFArrayGetCount(fields); index++) {
        CUIFieldRef field = (CUIFieldRef)CFArrayGetValueAtIndex(fields, index);
        CFStringRef messageText;
        struct pam_message *message;
        int messageStyle = 0;
        
        if (!_PAMMapCUIField(field, &messageStyle, &messageText))
            continue;

        message = (struct pam_message *)calloc(1, sizeof(*message));
        if (message == NULL) {
            rc = PAM_BUF_ERR;
            goto cleanup;
        }

        message->msg_style = messageStyle;
        if (messageText)
            message->msg = CUICFStringToCString(messageText);
        messages[cMessages++] = message;
    }
 
    rc = (conv->conv)((int)cMessages, (const struct pam_message **)messages, &resp, conv->appdata_ptr);
    if (rc != PAM_SUCCESS)
        goto cleanup;

    assert(resp);

    for (index = 0, cMessages = 0; index < cFields; index++) {
        CUIFieldRef field = (CUIFieldRef)CFArrayGetValueAtIndex(fields, index);
        struct pam_response *r;

        if (!_PAMMapCUIField(field, NULL, NULL))
            continue;

        r = &resp[cMessages++];

        if (r->resp) {
            CFStringRef value;

            value = CFStringCreateWithCString(kCFAllocatorDefault, r->resp, kCFStringEncodingUTF8);
            if (value) {
                CUIFieldSetValue(field, value);
                CFRelease(value);
            }
        }
    }

cleanup:
    _PAMConvFreeMessages(cMessages, messages);
    _PAMConvFreeResponses(cMessages, resp);

    return rc;
}

static int
_PAMSetTargetNameWithService(pam_handle_t *pamh, CUIControllerRef controller)
{
    const char *service = NULL;
    CFStringRef cfService;
    int rc;

    rc = pam_get_item(pamh, PAM_SERVICE, (const void **)&service);
    if (rc != PAM_SUCCESS)
        return rc;

    cfService = CFStringCreateWithCString(kCFAllocatorDefault, service, kCFStringEncodingUTF8);
    if (cfService == NULL)
        return PAM_BUF_ERR;

    CUIControllerSetTargetName(controller, cfService);
    CFRelease(cfService);

    return PAM_SUCCESS;
}

CUI_EXPORT int
pam_select_credential(pam_handle_t *pamh, int flags)
{
    int rc;
    CUIControllerRef controller = NULL;
    CFDictionaryRef attributes = NULL;
    CFDictionaryRef credAttributes = NULL;
    CFMutableArrayRef creds = NULL;
    CUICredentialRef selectedCred = NULL;
    char *user = NULL, *pass = NULL;
    Boolean autoLogin = false;
    __block CFIndex defaultCredentialIndex = kCFNotFound;

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

    rc = _PAMSetTargetNameWithService(pamh, controller);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
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
        
        rc = _PAMConvSelect(pamh, flags, creds, defaultCredentialIndex, &selectedCred);
        if (rc != PAM_SUCCESS)
            goto cleanup;
        
        CUICredentialDidBecomeSelected(selectedCred, &autoLogin);
        
        if (!autoLogin) {
            rc = _PAMConvCredential(pamh, flags, selectedCred);
            if (rc != PAM_SUCCESS)
                goto cleanup;
        }
        
        CUICredentialWillSubmit(selectedCred);
    } while (!CUICredentialCanSubmit(selectedCred));
    
    CUICredentialDidSubmit(selectedCred);
    
    credAttributes = CUICredentialGetAttributes(selectedCred);
    
    user = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrName));
    if (user) {
        rc = pam_set_item(pamh, PAM_USER, (const void *)user);
        if (rc != PAM_SUCCESS)
            goto cleanup;
    }
    
    pass = CUICFStringToCString((CFStringRef)CFDictionaryGetValue(credAttributes, kCUIAttrCredentialPassword));
    if (pass) {
        rc = pam_set_item(pamh, PAM_AUTHTOK, (const void *)pass);
        if (rc != PAM_SUCCESS)
            goto cleanup;
    }
    
    rc = pam_set_data(pamh, CREDUI_ATTR_DATA, (void *)CFRetain(credAttributes), _CUICleanupPAMAttrData);
    if (rc != PAM_SUCCESS)
        goto cleanup;
    
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
