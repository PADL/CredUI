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

/*
 * Helper functions for building attribute providers. Keeps anything GSS
 * item or keychain specific out of CredUICore.
 */

// 43022342-F1D5-424D-9D99-2973762F046D
#define kKeychainCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x43, 0x02, 0x23, 0x42, 0xF1, 0xD5, 0x42, 0x4D, 0x9D, 0x99, 0x29, 0x73, 0x76, 0x2F, 0x04, 0x6D)

// 2F62D1C1-F586-41CC-8096-C90683068DA5
#define kGSSItemCredentialProviderFactoryID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0x2F, 0x62, 0xD1, 0xC1, 0xF5, 0x86, 0x41, 0xCC, 0x80, 0x96, 0xC9, 0x06, 0x83, 0x06, 0x8D, 0xA5)

/*
 * Determine where attributes originated from. Useful for not creating
 * GSS items if they already exist.
 */
typedef CF_ENUM(CFIndex, CUIAttributeSource) {
    kCUIAttributeSourceUnknown = -1,
    kCUIAttributeSourceUser = 0,
    kCUIAttributeSourceGSSItem,
    kCUIAttributeSourceKeychain
};

extern CUIAttributeSource
CUIGetAttributeSource(CFDictionaryRef attributes);

CFStringRef
CUIGetDefaultUsername(CFDictionaryRef attributes);

Boolean
CUIShouldEnumerateForClass(CFDictionaryRef attributes, CFStringRef mechClass);

Boolean
CUIShouldEnumerateForPasswordClass(CFDictionaryRef attributes);

#endif /* defined(__CredUI__CUIProviderUtilities__) */
