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

/*
 * CUICredential type ID
 */
CUI_EXPORT CFTypeID
CUICredentialGetTypeID(void);

/*
 * Returns the fields associated with a credential. Each credential has an array of fields
 * that can be displayed to (and set by) the user in order to prepare the credential for
 * submission.
 */
CUI_EXPORT CFArrayRef
CUICredentialGetFields(CUICredentialRef cred);

/*
 * A helper function to find the first field of a given type.
 */
CUI_EXPORT CUIFieldRef
CUICredentialFindFirstFieldWithClass(CUICredentialRef cred, CUIFieldClass fieldClass);

/*
 * Returns a filtered list of fields matching the predicate block.
 */
CUI_EXPORT CFArrayRef
CUICredentialCopyFieldsWithPredicate(CUICredentialRef cred,
                                     Boolean (^predicate)(CUIFieldRef field));

/*
 * Returns the attributes associated with a credential. These attributes can be used to
 * authenticate the user, acquire a GSS credential, etc.
 */
CUI_EXPORT CFDictionaryRef
CUICredentialGetAttributes(CUICredentialRef cred);
  
/*
 * Notify the credential providers that a credential was selected by the user. If on
 * return *pbAutoLogin is true, then the credential should be used immediately.
 */  
CUI_EXPORT void
CUICredentialDidBecomeSelected(CUICredentialRef cred, Boolean *pbAutoLogin);

/*
 * Notify the credential provider that a credential was deselected by the user.
 */
CUI_EXPORT void
CUICredentialDidBecomeDeselected(CUICredentialRef cred);

/*
 * Notify the credential provider that the user is submitting the credential.
 */
CUI_EXPORT void
CUICredentialWillSubmit(CUICredentialRef cred);

/*
 * Returns true if the credential has all mandatory attributes and is ready for use.
 */
CUI_EXPORT Boolean
CUICredentialCanSubmit(CUICredentialRef cred);

/*
 * Notifies the credential provider that the user did submit a credential.
 */
CUI_EXPORT void
CUICredentialDidSubmit(CUICredentialRef cred);
 
/*
 * Apply a block to all fields in a credential.
 */   
CUI_EXPORT void
CUICredentialFieldsApplyBlock(CUICredentialRef cred, void (^cb)(CUIFieldRef, Boolean *stop));

/*
 * Ask the credential provider to persist this credential. If the credential is already
 * persisted, updates it.
 */
CUI_EXPORT Boolean
CUICredentialSavePersisted(CUICredentialRef cred, CFErrorRef *error);

/*
 * Ask the credential provider to delete this persisted credential. If the credential is not
 * persisted, behaviour is undefined.
 */
CUI_EXPORT Boolean
CUICredentialDeletePersisted(CUICredentialRef cred, CFErrorRef *error);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUICredential_h */
