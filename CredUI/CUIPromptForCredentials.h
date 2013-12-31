//
//  CUIPromptForCredentials.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__PromptForCredentials__
#define __CredUI__PromptForCredentials__

#include <CoreFoundation/CoreFoundation.h>

Boolean
CUIPromptForCredentials(CUICredUIContext *uiContext,
                        CFStringRef targetName,
                        CFTypeRef reserved,
                        CFErrorRef authError,
                        CFStringRef username,
                        CFStringRef password,
                        CFDictionaryRef *outCredAttributes,
                        Boolean *save,
                        CUIFlags flags);

#endif