//
//  CUIProviderUtilities.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIProviderUtilities__
#define __CredUI__CUIProviderUtilities__

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

extern CFDictionaryRef
CUICreateCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes);

extern CFDictionaryRef
CUICreateGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes);

#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIProviderUtilities__) */
