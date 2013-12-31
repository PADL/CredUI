//
//  CPersonaCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CPersonaCredentialProvider.h"
#include "CPersonaCredential.h"

Boolean CPersonaCredential::initWithControllerAndAttributes(
    CUIControllerRef controller,
    CFDictionaryRef attributes,
    CFErrorRef *error)
{
    CUIFieldRef fields[2] = { 0 };

    *error = NULL;
 
    gss_name_t gssTargetName = CUIControllerGetGssTargetName(controller);
    if (gssTargetName)
        _targetName = GSSNameCreateDisplayString(gssTargetName);

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
                                   CFErrorRef error;
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
    
    return true;
}

Boolean CPersonaCredential::createBrowserIDContext(CUIControllerRef controller, CFErrorRef *error)
{
    uint32_t ulContextFlags;

    *error = NULL;

    ulContextFlags = BID_CONTEXT_GSS | BID_CONTEXT_USER_AGENT | BID_CONTEXT_ECDH_KEYEX;

    _bidContext = BIDContextCreate(kCFAllocatorDefault, NULL, ulContextFlags, error);
    if (_bidContext) {
        BIDSetContextParam(_bidContext, BID_PARAM_ECDH_CURVE, (void *)BID_ECDH_CURVE_P521);

        const CUICredUIContext *uiContext = CUIControllerGetCredUIContext(controller);
        if (uiContext)
            BIDSetContextParam(_bidContext, BID_PARAM_PARENT_WINDOW, (void *)uiContext->parentWindow);
    }

    return !!_bidContext;
}

Boolean CPersonaCredential::createBrowserIDAssertion(CFErrorRef *error)
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
        CFDictionarySetValue(_attributes, kGSSAttrBrowserIDAssertion, assertion);
        CFDictionarySetValue(_attributes, kGSSAttrBrowserIDIdentity, identity);
        CFDictionarySetValue(_attributes, kCUICredentialStatus, kCUICredentialReturnCredentialFinished);
    } else {
        CFDictionarySetValue(_attributes, kCUICredentialStatus, kCUICredentialNotFinished);
    }
    
    return true;
}

