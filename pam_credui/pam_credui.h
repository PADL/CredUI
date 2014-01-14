//
//  pam_credui.h
//  CredUI
//
//  Created by Luke Howard on 14/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_pam_credui_h
#define CredUI_pam_credui_h

#include <security/pam_appl.h>

#ifdef __cplusplus
extern "C" {
#endif
    
CUI_EXPORT int
pam_select_credential(pam_handle_t *pamh);

#ifdef __cplusplus
}
#endif

#endif /* CredUI_pam_credui_h */
