//
//  CustomField.h
//  CredUI
//
//  Created by Luke Howard on 7/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CustomField__
#define __CredUI__CustomField__

#include <CredUICore/CredUICore.h>

#include "SampleCredential.h"

/*
 * A custom field for setting the username.
 */
CUIFieldRef
CustomFieldCreate(SampleCredential *cred);

#endif /* defined(__CredUI__CustomField__) */
