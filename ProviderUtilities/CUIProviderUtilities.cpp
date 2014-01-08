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

CUIClassMatchResult
CUIShouldEnumerateForClass(CFDictionaryRef attributes, CFStringRef assertedClass)
{
    CFStringRef attrClass = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass);

    if (attrClass == NULL)
        return CUIClassAbsent;

    return CFEqual(attrClass, assertedClass) ? CUIClassMatch : CUIClassMismatch;
}

static CFStringRef
_CUIDefaultPasswordCapableClasses[] = {
    kCUIAttrClassKerberos,
    kCUIAttrClassNTLM,
    kCUIAttrClassIAKerb,
    kCUIAttrClassGeneric
};

CUIClassMatchResult
CUIShouldEnumerateForPasswordClass(CUIControllerRef controller,
                                   CUIUsageFlags usageFlags,
                                   CFDictionaryRef attributes)
{
    CFStringRef attrClass = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass);
    CFIndex i;
    CUIClassMatchResult ret = CUIClassMismatch;

    if (usageFlags & kCUIUsageFlagsRequireCertificates)
        return CUIClassMismatch;

    if (attrClass == NULL)
        return CUIClassAbsent;

    for (i = 0; i < sizeof(_CUIDefaultPasswordCapableClasses) / sizeof(_CUIDefaultPasswordCapableClasses[0]); i++) {
        CFStringRef thisClass =  _CUIDefaultPasswordCapableClasses[i];

        if (CUIShouldEnumerateForClass(attributes, thisClass)) {
            ret = CUIClassMatch;
            break;
        }
    }

    return ret;
}

Boolean
CUIIsPersistedCredential(CFDictionaryRef attributes)
{
    CFUUIDRef factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrPersistenceFactoryID);

    return factoryID && CFGetTypeID(factoryID) == CFUUIDGetTypeID();
}
