//
//  CUIProviderUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"
#include "GSSItem.h"

CFStringRef
CUIGetDefaultUsername(CFDictionaryRef attributes)
{
    CFStringRef defaultUsername;

    defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameDisplay);
    if (defaultUsername == NULL) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);
    }
    
    return defaultUsername;
}

Boolean
CUIShouldEnumerateForClass(CFDictionaryRef attributes, CFStringRef mechClass)
{
    CFStringRef attrClass = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass);
    
    if (attrClass == NULL)
        return TRUE;
    
    return CFEqual(attrClass, mechClass);
}

Boolean
CUIShouldEnumerateForPasswordClass(CFDictionaryRef attributes)
{
    return CUIShouldEnumerateForClass(attributes, kCUIAttrClassKerberos) ||
           CUIShouldEnumerateForClass(attributes, kCUIAttrClassNTLM) ||
           CUIShouldEnumerateForClass(attributes, kCUIAttrClassIAKerb) ||
           CUIShouldEnumerateForClass(attributes, kCUIAttrClassGeneric);
}

Boolean
CUIIsPersistedCredential(CFDictionaryRef attributes)
{
    CFUUIDRef factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrPersistenceFactoryID);

    return factoryID && CFGetTypeID(factoryID) == CFUUIDGetTypeID();
}
