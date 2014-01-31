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

CFStringRef
_CUICopyAttrClassForAuthError(CFErrorRef authError)
{
    CFStringRef mechClass = NULL;
    
    if (!CUIIsGSSError(authError))
        return NULL;
    
    CFDictionaryRef userInfo = CFErrorCopyUserInfo(authError);
    if (userInfo) {
        CFStringRef mechOid = (CFStringRef)CFDictionaryGetValue(userInfo, kGSSMechanismOID);
        CFStringRef mechName = (CFStringRef)CFDictionaryGetValue(userInfo, kGSSMechanism);
        
        mechClass = CUIAttrClassForMech(mechName, mechOid);
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
    
    if (!CUIIsGSSError(authError))
        return false;
    
    major = CFErrorGetCode(authError);
    
    return (GSS_ROUTINE_ERROR(major) == GSS_S_NO_CRED ||
            (major & GSS_S_PROMPTING_NEEDED));
    
}
