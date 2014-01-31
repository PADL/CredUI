//
//  CUICFUtilities.h
//  CredUI
//
//  Created by Luke Howard on 14/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CUICFUtilities_h
#define CredUI_CUICFUtilities_h

#include <CoreFoundation/CFString.h>

char *
CUICFStringToCString(CFStringRef string);

CFErrorRef
CUICFErrorCreate(CFIndex errorCode);

void
CUICFErrorComplete(void (^completionHandler)(CFErrorRef), OSStatus errorCode);

#endif /* CredUI_CUICFUtilities_h */
