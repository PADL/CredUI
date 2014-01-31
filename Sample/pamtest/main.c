//
//  main.c
//  CredUI
//
//  Created by Luke Howard on 14/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

/*
 * Usage:
 *
 * pamtest [user] [service]
 * where:
 *    user is login name if not specified
 *    service is "login" if not specified
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>

#include <security/pam_appl.h>

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

int pamtestConv(int num_msg,
                const struct pam_message **msgv,
                struct pam_response **respv,
                void *appdata_ptr)
{
    int i;
    char buf[PAM_MAX_MSG_SIZE];
    
    fprintf(stderr, "%s:%d pamtestConv num_msg=%d\n",
            __FILE__, __LINE__, num_msg);
    
    *respv = (struct pam_response *)calloc(num_msg, sizeof(struct pam_response));
    if (*respv == NULL) {
        return PAM_BUF_ERR;
    }
    
    for (i = 0; i < num_msg; i++) {
        const struct pam_message *msg = msgv[i];
        struct pam_response *resp = &((*respv)[i]);
        
        char *p = NULL;
        
        assert(msg);

        switch (msg->msg_style) {
            case PAM_PROMPT_ECHO_OFF:
                p = getpass(msg->msg);
                break;
            case PAM_PROMPT_ECHO_ON:
                printf("%s", msg->msg);
                p = fgets(buf, sizeof(buf), stdin);
                if (p != NULL) p[strlen(p) - 1] = '\0';
                break;
            case PAM_ERROR_MSG:
                fprintf(stderr, "%s\n", msg->msg);
                break;
            case PAM_TEXT_INFO:
#ifdef PAM_MSG_NOCONF
            case PAM_MSG_NOCONF:
#endif
                printf("%s\n", msg->msg);
                break;
            default:
#ifdef PAM_CONV_INTERRUPT
            case PAM_CONV_INTERRUPT:
#endif
                return PAM_CONV_ERR;
        }
        
        resp->resp = (p == NULL) ? NULL : strdup(p);
        resp->resp_retcode = 0;
    }
    
    return PAM_SUCCESS;
}

int main(int argc, char *argv[])
{
    char *user = (argc > 1) ? argv[1] : getlogin();
    char *service = (argc > 2) ? argv[2] : "pamtest";
    struct pam_conv conv;
    pam_handle_t *pamh = NULL;
    int rc;
    
    conv.conv = pamtestConv;
    conv.appdata_ptr = NULL;
    
    fprintf(stderr, "%s:%d Starting with user=%s service=%s\n",
            __FILE__, __LINE__, user, service);
    
    rc = pam_start(service, user, &conv, &pamh);
    CHECK_STATUS(pamh, "pam_start", rc);
    
retry:
    rc = pam_select_credential(pamh, 0);
    CHECK_STATUS(pamh, "pam_select_credential", rc);
    
    rc = pam_authenticate(pamh, 0);
    if (rc == PAM_TRY_AGAIN)
        goto retry;
    
    CHECK_STATUS(pamh, "pam_authenticate", rc);
    
    rc = pam_acct_mgmt(pamh, 0);
    CHECK_STATUS(pamh, "pam_acct_mgmt", rc);
    
    if (rc == PAM_SUCCESS) {
        rc = pam_open_session(pamh, 0);
        CHECK_STATUS(pamh, "pam_open_session", rc);
        
        rc = pam_close_session(pamh, 0);
        CHECK_STATUS(pamh, "pam_close_session", rc);
    }
    
cleanup:    
    if (pamh != NULL)
        pam_end(pamh, rc);
    
    return rc;
}

