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

/*
 * Helper functions for building attribute providers. Keeps anything GSS
 * item or keychain specific out of CredUICore.
 */

/*
 * Return an attribute dictionary suitable for passing to a GSS item API
 * from a CredUI attribute dictionary.
 */
extern CFMutableDictionaryRef
CUICreateCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes);

/*
 * Return an attribute dictionary suitable for passing to a CredUI API
 * from a GSS item attribute dictionary.
 */
extern CFMutableDictionaryRef
CUICreateGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes);

/*
 * Determine where attributes originated from. Useful for not creating
 * GSS items if they already exist.
 */
typedef CF_ENUM(CFIndex, CUIAttributeSource) {
    kCUIAttributeSourceUser = 0,
    kCUIAttributeSourceGSSItem,
    kCUIAttributeSourceKeychain
};

extern CUIAttributeSource
CUIGetAttributeSource(CFDictionaryRef attributes);

extern Boolean
CUIGSSItemAddOrUpdate(CFDictionaryRef attributes, Boolean addOnly, CFErrorRef *error);

extern CFArrayRef
CUIKeychainCopyMatching(CFDictionaryRef attributes,
                        CFTypeRef targetName,
                        CFErrorRef *error);

extern CFMutableDictionaryRef
CUICreateKeychainAttributesFromCUIAttributes(CFDictionaryRef attributes, CFTypeRef targetName, Boolean *pbCUIGeneric);

extern CFMutableDictionaryRef
CUICreateCUIAttributesFromKeychainAttributes(CFDictionaryRef keychainAttrs, Boolean bCUIGeneric);
    
extern Boolean
CUIKeychainSetPasswordAttr(CFMutableDictionaryRef keychainAttrs,
                           CFDictionaryRef attributes);

extern Boolean
CUIKeychainStore(CFDictionaryRef attributes,
                 CFTypeRef targetName,
                 CFErrorRef *error);
    
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIProviderUtilities__) */
