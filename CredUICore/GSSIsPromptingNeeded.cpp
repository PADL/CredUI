//
//  GSSIsPromptingNeeded.cpp
//  CredUI
//
//  Created by Luke Howard on 9/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#ifndef GSS_S_PROMPTING_NEEDED
#define GSS_S_PROMPTING_NEEDED (1 << (GSS_C_SUPPLEMENTARY_OFFSET + 5))
#endif

static Boolean
_CUIIsGSSError(CFErrorRef error)
{
    CFStringRef domain = error ? CFErrorGetDomain(error) : NULL;
    
    return domain && CFEqual(domain, CFSTR("org.h5l.GSS"));
}

/*
 * XXX This unfortunately is very brittle.
 */
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

CFStringRef
_CUICopyAttrClassForAuthError(CFErrorRef authError)
{
    CFStringRef mechClass = NULL;
    
    if (!_CUIIsGSSError(authError))
        return NULL;
    
    CFDictionaryRef userInfo = CFErrorCopyUserInfo(authError);
    if (userInfo) {
        CFStringRef mechOid = (CFStringRef)CFDictionaryGetValue(userInfo, CFSTR("kGSSMechanismOID"));
        CFStringRef mechName = (CFStringRef)CFDictionaryGetValue(userInfo, CFSTR("kGSSMechanism"));
        
        mechClass = _CUIAttrClassForMech(mechName, mechOid);
        if (mechClass)
            CFRetain(mechClass);
        
        CFRelease(userInfo);
    }
    
    return mechClass;
}

CUI_EXPORT Boolean
GSSIsPromptingNeeded(CFErrorRef authError)
{
    CFIndex major;
    
    if (!_CUIIsGSSError(authError))
        return false;
    
    major = CFErrorGetCode(authError);
    
    return (GSS_ROUTINE_ERROR(major) == GSS_S_NO_CRED ||
            (major & GSS_S_PROMPTING_NEEDED));
    
}
