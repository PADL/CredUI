//
//  CUIProviderUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <GSS/GSS.h>
#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"

CFStringRef
CUICopyDefaultUsername(CFDictionaryRef attributes)
{
    CFTypeRef name = CFDictionaryGetValue(attributes, kCUIAttrName);
    CFTypeRef defaultUsername = CFDictionaryGetValue(attributes, kCUIAttrNameDisplay);
    
    if (defaultUsername) {
        CFRetain(defaultUsername);
    } else if (name) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);

        if (nameType == NULL ||
            CFEqual(nameType, kCUIAttrNameTypeGSSUsername) ||
            CFEqual(nameType, kCUIAttrNameTypeGSSHostBasedService) ||
            CFEqual(nameType, kCUIAttrNameTypePosixName)) {
            defaultUsername = CFRetain(name);
        } else if (CFEqual(nameType, kCUIAttrNameTypeGSSExportedName)) {
            gss_name_t gssName = GSSCreateName(name, GSS_C_NT_EXPORT_NAME, NULL);

            if (gssName) {
                defaultUsername = GSSNameCreateDisplayString(gssName);
                CFRelease(gssName);
            }
        }
    }
   
    if (defaultUsername && CFGetTypeID(defaultUsername) != CFStringGetTypeID()) {
        CFRelease(defaultUsername);
        return NULL;
    }
 
    return (CFStringRef)defaultUsername;
}

Boolean
CUIIsPersistedCredential(CFDictionaryRef attributes)
{
    CFUUIDRef factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrPersistenceFactoryID);

    return factoryID && CFGetTypeID(factoryID) == CFUUIDGetTypeID();
}

Boolean
CUIIsIdentityCredential(CFDictionaryRef attributes)
{
    CFUUIDRef factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrIdentityFactoryID);

    return factoryID && CFGetTypeID(factoryID) == CFUUIDGetTypeID();
}