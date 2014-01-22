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
    CFStringRef attrClass = attributes ? (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass) : NULL;

    if (usageFlags & (kCUIUsageFlagsGeneric | kCUIUsageFlagsExcludeTransientCreds))
        return NULL;

    mech = CUICopyGSSOIDForAttrClass(attrClass, mechBuf);

    gss_iter_creds(&minor, 0, mech, ^(gss_iter_OID oid, gss_cred_id_t gssCred) {
        CUICredentialRef cred = CUIGSSCredCredentialCreate(CFGetAllocator(_controller), gssCred);
        if (cred) {
            CFArrayAppendValue(creds, cred);
            CFRelease(cred);
        }
    });

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
