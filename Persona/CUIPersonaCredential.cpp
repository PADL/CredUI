//
//  CUIPersonaCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <GSS/GSS.h>

#include "CUIPersonaCredentialProvider.h"
#include "CUIPersonaCredential.h"
#include "CUIProviderUtilities.h"

static gss_OID_desc GSSBrowserIDAes128MechDesc =
{ 10, (void *)"\x2B\x06\x01\x04\x01\xA9\x4A\x18\x01\x11" };

static CFStringRef
kPersonaCredentialProvider = CFSTR("com.padl.CredUI.Providers.PersonaCredentialProvider");

static CFStringRef CUIPersonaMakeGssTargetName(gss_name_t name)
{
    OM_uint32 major, minor;
    gss_name_t servicePrincipalName;
    CFStringRef displayName;
    
    major = gss_canonicalize_name(&minor, name, &GSSBrowserIDAes128MechDesc, &servicePrincipalName);
    if (GSS_ERROR(major))
        return NULL;
    
    displayName = GSSNameCreateDisplayString(servicePrincipalName);
    
    CFRelease(servicePrincipalName);
    
    return displayName;
}

Boolean CUIPersonaCredential::initWithControllerAndAttributes(
    CUIControllerRef controller,
    CUIUsageFlags usageFlags,
    CFDictionaryRef attributes,
    CFErrorRef *error)
{
    CUIFieldRef fields[2] = { 0 };

    if (error != NULL)
        *error = NULL;

    CFTypeRef targetName = CUIControllerGetTargetName(controller);
    if (targetName) {
        // turn on GSS flags if we have a GSS target name
        _targetName = CUIPersonaMakeGssTargetName((gss_name_t)targetName);
        if (_targetName != NULL)
            _contextFlags |= BID_CONTEXT_GSS | BID_CONTEXT_ECDH_KEYEX;
        else
            _targetName = CUICopyTargetDisplayName(targetName);
        
        if (_targetName == NULL)
            return false;
    }

    if (!createBrowserIDContext(controller, error))
        return false;

    if (attributes) {
        /*
         * We can't do anything with stored GSSItems
         */
        if (CUIIsPersistedCredential(attributes))
            return false;
        
        CFTypeRef attrClass = CFDictionaryGetValue(attributes, kCUIAttrClass);
        if ((usageFlags & kCUIUsageFlagsMechanismOnly) &&
            attrClass &&
            !CFEqual(attrClass, kCUIAttrClassBrowserID))
            return false;
        
        _defaultIdentity = CUIGetDefaultUsername(attributes);
        if (_defaultIdentity)
            CFRetain(_defaultIdentity);        
    }

    fields[0] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Sign in with Persona"), NULL);
    fields[1] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, CFSTR("Submit"), NULL,
                               ^(CUIFieldRef field, CFTypeRef value) {
                                   // this is the willSubmit callback; CredUI only renders a single submit button
                                   // for the network UI case
                                   createBrowserIDAssertion(NULL);
    });
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, sizeof(fields) / sizeof(fields[0]), &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        return false;

    _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (_attributes == NULL)
        return false;
    
    CFDictionarySetValue(_attributes, kCUIAttrClass, kCUIAttrClassBrowserID);
    CFDictionarySetValue(_attributes, kCUIAttrSupportGSSCredential, kCFBooleanTrue);
    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider, kPersonaCredentialProvider);
    CFDictionarySetValue(_attributes, kCUIAttrProviderFactoryID, kPersonaCredentialProviderFactoryID);

    return true;
}

Boolean CUIPersonaCredential::createBrowserIDContext(CUIControllerRef controller, CFErrorRef *error)
{
    if (error != NULL)
        *error = NULL;

    _bidContext = BIDContextCreate(CFGetAllocator(controller), NULL, _contextFlags, error);
    if (_bidContext == NULL)
        return false;
    
    const CUICredUIContext *uiContext = CUIControllerGetCredUIContext(controller);
    if (uiContext)
        BIDSetContextParam(_bidContext, BID_PARAM_PARENT_WINDOW, (void *)uiContext->parentWindow);

    return true;
}

Boolean CUIPersonaCredential::createBrowserIDAssertion(CFErrorRef *error)
{
    CFStringRef assertion;
    BIDIdentity identity = BID_C_NO_IDENTITY;
    uint32_t ulReqFlags, ulRetFlags = 0;

    if (error != NULL)
        *error = NULL;
    
    ulReqFlags = BID_ACQUIRE_FLAG_NO_CACHED;
    if (_contextFlags & BID_CONTEXT_GSS)
        ulReqFlags |= BID_ACQUIRE_FLAG_MUTUAL_AUTH;

    assertion = BIDAssertionCreateUI(_bidContext,
                                     _targetName,
                                     NULL, /* channelBindings */
                                     _defaultIdentity,
                                     ulReqFlags,
                                     &identity,
                                     &ulRetFlags,
                                     error);
    if (assertion) {
        ulRetFlags |= ulReqFlags;
        
        CFNumberRef bidFlags = CFNumberCreate(CFGetAllocator(assertion), kCFNumberSInt32Type, (void *)&ulRetFlags);

        CFDictionarySetValue(_attributes, kCUIAttrCredentialBrowserIDAssertion, assertion);
        CFDictionarySetValue(_attributes, kCUIAttrCredentialBrowserIDIdentity, identity);
        CFDictionarySetValue(_attributes, kCUIAttrCredentialBrowserIDFlags, bidFlags);
        CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, kCUICredentialReturnCredentialFinished);
        
        CFStringRef subject = (CFStringRef)BIDIdentityCopyAttributeValue(identity, kBIDIdentitySubjectKey);
        
        CFDictionarySetValue(_attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername);
        CFDictionarySetValue(_attributes, kCUIAttrName, subject);
        
        CFRelease(bidFlags);
        CFRelease(assertion);
        CFRelease(identity);
        CFRelease(subject);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrCredentialStatus, kCUICredentialNotFinished);
    }
    
    return true;
}

