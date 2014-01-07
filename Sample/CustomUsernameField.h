//
//  CustomField.h
//  CredUI
//
//  Created by Luke Howard on 7/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CustomUsernameField__
#define __CredUI__CustomUsernameField__

#include <CredUICore/CredUICore.h>

#include "SampleCredential.h"

/*
 * A custom field for setting the username.
 */
CUIFieldRef
CustomUsernameFieldCreate(SampleCredential *cred);

#endif /* defined(__CredUI__CustomUsernameField__) */
