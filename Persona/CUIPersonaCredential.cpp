
//
//  CUIPersonaCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIPersonaCredentialProvider.h"
#include "CUIPersonaCredential.h"

static const gss_OID_desc GSSBrowserIDAes128MechDesc =
{ 10, (void *)"\x2B\x06\x01\x04\x01\xA9\x4A\x18\x01\x11" };

static CFStringRef CUIPersonaCreateTargetName(gss_name_t name)
{
    OM_uint32 major, minor;
    gss_name_t kerbName;
    CFStringRef displayName;
    
    // for production we need to canon using the BrowserID not Kerberos
#if 0
    major = gss_canonicalize_name(&minor, name, &GSSBrowserIDAes128MechDesc, &kerbName);
#else
    major = gss_canonicalize_name(&minor, name, GSS_KRB5_MECHANISM, &kerbName);
#endif
    if (GSS_ERROR(major))
        return NULL;
    
    displayName = GSSNameCreateDisplayString(kerbName);
    
    CFRelease(kerbName);
    
    return displayName;
}

Boolean CUIPersonaCredential::initWithControllerAndAttributes(
    CUIControllerRef controller,
    CFDictionaryRef attributes,
    CFErrorRef *error)
{
    CUIFieldRef fields[2] = { 0 };

    *error = NULL;
 
    gss_name_t gssTargetName = CUIControllerGetGssTargetName(controller);
    if (gssTargetName) {
        _targetName = CUIPersonaCreateTargetName(gssTargetName);
        if (_targetName == NULL)
            return false; // could mean BrowserID is not installed on this system
    }

    if (!createBrowserIDContext(controller, error))
        return false;

    if (attributes) {
        /*
         * We can't do anything with stored GSSItems
         */
        if (CFDictionaryGetValue(attributes, kGSSAttrUUID))
            return false;
        
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kGSSAttrNameType);
        CFStringRef name = (CFStringRef)CFDictionaryGetValue(attributes, kGSSAttrName);

        if (nameType && CFEqual(nameType, kGSSAttrNameTypeGSSUsername) && name)
            _defaultIdentity = (CFStringRef)CFRetain(name);
    }

    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, CFSTR("Sign in with Persona"), NULL, NULL);
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, CFSTR("Submit"), NULL,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   // this is the willSubmit callback; CredUI only renders a single submit button
                                   // for the network UI case
                                   CFErrorRef error = NULL;
                                   
                                   createBrowserIDAssertion(&error);
                                   
                                   if (error)
                                       CFRelease(error);

    });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;

    _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (_attributes == NULL)
        return false;
    
    CFDictionarySetValue(_attributes, kGSSAttrClass, kGSSAttrClassBrowserID);
    
    return true;
}

void CUIPersonaCredential::didSubmit(void)
{
}

Boolean CUIPersonaCredential::createBrowserIDContext(CUIControllerRef controller, CFErrorRef *error)
{
    uint32_t ulContextFlags;

    *error = NULL;

    ulContextFlags = BID_CONTEXT_GSS | BID_CONTEXT_USER_AGENT | BID_CONTEXT_ECDH_KEYEX;

    _bidContext = BIDContextCreate(kCFAllocatorDefault, NULL, ulContextFlags, error);
    if (_bidContext) {
        BIDSetContextParam(_bidContext, BID_PARAM_ECDH_CURVE, (void *)BID_ECDH_CURVE_P521);

#if 1
        const CUICredUIContext *uiContext = CUIControllerGetCredUIContext(controller);
        if (uiContext)
            BIDSetContextParam(_bidContext, BID_PARAM_PARENT_WINDOW, (void *)uiContext->parentWindow);
#endif
    }

    return !!_bidContext;
}

Boolean CUIPersonaCredential::createBrowserIDAssertion(CFErrorRef *error)
{
    CFStringRef assertion;
    BIDIdentity identity = BID_C_NO_IDENTITY;
    uint32_t ulReqFlags, ulRetFlags = 0;

    *error = NULL;
    
    ulReqFlags = BID_ACQUIRE_FLAG_NO_CACHED | BID_ACQUIRE_FLAG_MUTUAL_AUTH;

    assertion = BIDAssertionCreateUI(_bidContext,
                                     _targetName,
                                     NULL, /* channelBindings */
                                     _defaultIdentity,
                                     ulReqFlags,
                                     &identity,
                                     &ulRetFlags,
                                     error);
    if (assertion) {
        CFDictionarySetValue(_attributes, kGSSAttrCredentialBrowserIDAssertion, assertion);
        CFDictionarySetValue(_attributes, kGSSAttrCredentialBrowserIDIdentity, identity);
        CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, kCUICredentialReturnCredentialFinished);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, kCUICredentialNotFinished);
    }
    
    return true;
}

