//
//  CUIGSSCredCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 21/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIGSSCredCredentialProvider.h"
#include "CUIGSSCredCredential.h"
#include "CUIGSSUtilities.h"

extern "C" {
    void *CUIGSSCredCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
}

CFArrayRef
CUIGSSCredCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
                                                      CUIUsageFlags usageFlags,
                                                      CFIndex *defaultCredentialIndex,
                                                      CFErrorRef *error)
{
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
    OM_uint32 minor;
    gss_OID mech = GSS_C_NO_OID;
    gss_OID_desc mechBuf = { 0 };
    gss_name_t desiredName = GSS_C_NO_NAME;
    CFStringRef attrClass = attributes ? (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass) : NULL;

    if (usageFlags & (kCUIUsageFlagsGeneric | kCUIUsageFlagsExcludeTransientCreds))
        return NULL;

    if (attributes)
        desiredName = CUICopyGSSNameForAttributes(attributes);
    
    mech = CUICopyGSSOIDForAttrClass(attrClass, mechBuf);

    gss_iter_creds(&minor, 0, mech, ^(gss_iter_OID oid, gss_cred_id_t gssCred) {
        if (desiredName) {
            OM_uint32 tmpMinor;
            gss_name_t credName;
            int nameEqual = 0;

            credName = GSSCredentialCopyName(gssCred);
            if (credName) {
                gss_compare_name(&tmpMinor, credName, desiredName, &nameEqual);
                CFRelease(credName);
            }
            
            if (!nameEqual)
                return;
        }
        
        CUICredentialRef cred = CUIGSSCredCredentialCreate(CFGetAllocator(_controller), gssCred);
        if (cred) {
            CFArrayAppendValue(creds, cred);
            CFRelease(cred);
        }
    });
    
    if (desiredName)
        CFRelease(desiredName);

    return creds;
}

Boolean CUIGSSCredCredentialProvider::initWithController(CUIControllerRef controller,
                                                         CFErrorRef *error)
{
    if (CUIControllerGetUsageScenario(controller) != kCUIUsageScenarioNetwork)
        return false;

    _controller = (CUIControllerRef)CFRetain(controller);
    
    return true;
}


__attribute__((visibility("default"))) void *
CUIGSSCredCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIGSSCredCredentialProvider;
    
    return NULL;
}
