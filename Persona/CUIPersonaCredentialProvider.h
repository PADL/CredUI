//
//  PersonaCredentialProvider.h
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__PersonaCredentialProvider__
#define __CredUI__PersonaCredentialProvider__

#include "CredUICore.h"

class PersonaCredentialProvider;

extern const CFStringRef kCUIAttrCredentialBrowserIDAssertion;
extern const CFStringRef kCUIAttrCredentialBrowserIDIdentity;
extern const CFStringRef kCUIAttrCredentialBrowserIDFlags;

extern const CFStringRef kCUIAttrClassBrowserID;

// 58733A29-A6A5-4E57-93EB-200D9411F686
#define kPersonaCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x58, 0x73, 0x3A, 0x29, 0xA6, 0xA5, 0x4E, 0x57, 0x93, 0xEB, 0x20, 0x0D, 0x94, 0x11, 0xF6, 0x86)

#endif /* defined(__CredUI__PersonaCredentialProvider__) */
