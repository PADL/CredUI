//
//  CUIPersistedCredential.h
//  CredUI
//
//  Created by Luke Howard on 15/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CUIPersistedCredential_h
#define CredUI_CUIPersistedCredential_h

#include <CoreFoundation/CoreFoundation.h>
#include <libkern/OSAtomic.h>

#include <Security/Security.h>

#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"

class CUICredentialPersistenceEx : public CUICredentialPersistence {
public:
    virtual Boolean updateCredential(CUICredentialRef credential, CFErrorRef *error) = 0;
    virtual Boolean deleteCredential(CUICredentialRef credential, CFErrorRef *error) = 0;
};

/*
 * Wrap a credential with updating hooks, to be returned from persistence providers.
 */
CUICredentialRef
CUIPersistedCredentialCreate(CUICredentialPersistenceEx *provider, CUICredentialRef cred);

#endif /* CredUI_CUIPersistedCredential_h */
