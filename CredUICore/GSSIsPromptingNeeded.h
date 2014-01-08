//
//  GSSIsPromptingNeeded.h
//  CredUI
//
//  Created by Luke Howard on 8/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_GSSIsPromptingNeeded_h
#define CredUICore_GSSIsPromptingNeeded_h

#include <CoreFoundation/CFError.h>

#ifdef __cplusplus
extern "C" {
#endif

CUI_EXPORT Boolean
GSSIsPromptingNeeded(CFErrorRef authError);

#ifdef __cplusplus
}
#endif

#endif
