//
//  CUICredential.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUICredential_h
#define CredUICore_CUICredential_h

#include <CredUICore/CUIBase.h>
#include <CredUICore/CUIField.h>

#ifdef __cplusplus
extern "C" {
#endif
    
struct __CUICredential;
    
typedef struct __CUICredential *CUICredentialRef;

CUI_EXPORT CFTypeID
CUICredentialGetTypeID(void);    
    
CUI_EXPORT CFArrayRef
CUICredentialGetFields(CUICredentialRef cred);

CUI_EXPORT CUIFieldRef
CUICredentialFindFirstFieldWithClass(CUICredentialRef cred, CUIFieldClass fieldClass);
    
CUI_EXPORT CFArrayRef
CUICredentialCopyFieldsWithPredicate(CUICredentialRef cred,
                                     Boolean (^predicate)(CUIFieldRef field));

CUI_EXPORT CFDictionaryRef
CUICredentialGetAttributes(CUICredentialRef cred);
    
CUI_EXPORT void
CUICredentialDidBecomeSelected(CUICredentialRef cred, Boolean *pbAutoLogin);

CUI_EXPORT void
CUICredentialDidBecomeDeselected(CUICredentialRef cred);

/*
 * Call this once the user has selected a credential and you want to use it
 */
CUI_EXPORT void
CUICredentialWillSubmit(CUICredentialRef cred);

/*
 * Call CUICredentialCanSubmit after CUICredentialWillSubmit to validate
 * the credential has any mandatory attributes.
 */
CUI_EXPORT Boolean
CUICredentialCanSubmit(CUICredentialRef cred);

/*
 * Call CUICredentialDidSubmit after the credential has been submitted to
 * notify the provider.
 */
CUI_EXPORT void
CUICredentialDidSubmit(CUICredentialRef cred);
    
CUI_EXPORT void
CUICredentialFieldsApplyBlock(CUICredentialRef cred, void (^cb)(CUIFieldRef, Boolean *stop));

CUI_EXPORT Boolean
CUICredentialSavePersisted(CUICredentialRef cred, CFErrorRef *error);

CUI_EXPORT Boolean
CUICredentialDeletePersisted(CUICredentialRef cred, CFErrorRef *error);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUICredential_h */
