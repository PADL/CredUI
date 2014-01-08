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

#if 0
static CFStringRef
_CUIAttrClassForMech(CFStringRef errMechName, CFStringRef errMechOid)
{
    if (errMechName) {
        if (CFEqual(errMechName, CFSTR("krb5")))
            return kCUIAttrClassKerberos;
        else if (CFEqual(errMechName, CFSTR("ntlm")))
            return kCUIAttrClassNTLM;
        else if (CFEqual(errMechName, CFSTR("iakerb")))
            return kCUIAttrClassIAKerb;
    }

    return errMechOid;
}

Boolean
_CUIIsGSSError(CFErrorRef error)
{
    CFStringRef domain = error ? CFErrorGetDomain(error) : NULL;

    return domain && CFEqual(domain, CFSTR("org.h5l.GSS"));
}

CUIClassMatchResult
CUIAuthErrorMatchesClass(CUIControllerRef controller, CFStringRef assertedClass)
{
    CFErrorRef authError = CUIControllerGetAuthError(controller);
    CUIClassMatchResult ret = CUIClassAbsent;

    if (_CUIIsGSSError(authError)) {
        CFDictionaryRef userInfo = CFErrorCopyUserInfo(authError);

        if (userInfo) {
            CFStringRef mechOid = (CFStringRef)CFDictionaryGetValue(userInfo, CFSTR("kGSSMechanismOID"));
            CFStringRef mechName = (CFStringRef)CFDictionaryGetValue(userInfo, CFSTR("kGSSMechanism"));
            CFStringRef mechClass = _CUIAttrClassForMech(mechName, mechOid);

            if (mechClass)
                ret = CFEqual(mechClass, assertedClass) ? CUIClassMatch : CUIClassMismatch;

            CFRelease(userInfo);
        }
    }

    return ret;
}
#endif

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
