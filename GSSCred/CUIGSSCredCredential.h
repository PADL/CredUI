//
//  CUIGSSCredCredential.h
//  CredUI
//
//  Created by Luke Howard on 21/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIGSSCredCredential__
#define __CredUI__CUIGSSCredCredential__

#include <GSS/GSS.h>
#include <CredUICore/CredUICore.h>

CUICredentialRef
CUIGSSCredCredentialCreate(CFAllocatorRef allocator, gss_cred_id_t gssCred);

#endif /* defined(__CredUI__CUIGSSCredCredential__) */
