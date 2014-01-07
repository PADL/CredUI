//
//  CUICertificateCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <Security/Security.h>

#include "CUICertificateCredentialProvider.h"
#include "CUICertificateCredential.h"
#include "CUIProviderUtilities.h"

static CFStringRef
kCertificateCredentialProvider = CFSTR("com.padl.CredUI.Providers.CertificateCredentialProvider");

extern "C" {
    CFStringRef
    _CSCopyAppleIDAccountForAppleIDCertificate(SecCertificateRef, CFErrorRef *);

    CFStringRef
    _CSCopyKerberosPrincipalForCertificate(SecCertificateRef);
};

Boolean
CUICertificateCredential::initWithSecIdentity(SecIdentityRef identity,
                                              CUIUsageFlags usageFlags,
                                              CFErrorRef *error)
{
    Boolean ret = false;
    SecCertificateRef cert = NULL;
    CFStringRef appleIDName = NULL;
    CFStringRef displayName = NULL;
    CFStringRef kerberosName = NULL;
    CUIFieldRef fields[3] = { 0 };
    size_t cFields = 0;
    
    if (error != NULL)
        *error = NULL;
   
    _attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (_attributes == NULL)
        goto cleanup;
    
    if (SecIdentityCopyCertificate(identity, &cert) != errSecSuccess)
        goto cleanup;

    CFDictionarySetValue(_attributes, kCUIAttrCredentialProvider,       kCertificateCredentialProvider);
    CFDictionarySetValue(_attributes, kCUIAttrProviderFactoryID,        kCertificateCredentialProviderFactoryID);
    CFDictionarySetValue(_attributes, kCUIAttrNameType,                 kCUIAttrNameTypeGSSUsername);
    CFDictionarySetValue(_attributes, kCUIAttrCredentialExists,         kCFBooleanTrue);
    if (usageFlags & kCUIUsageFlagsGeneric) {
        CFDictionarySetValue(_attributes, kCUIAttrClass,                kCUIAttrClassGeneric);
    } else {
        CFDictionarySetValue(_attributes, kCUIAttrSupportGSSCredential, kCFBooleanTrue);
    }
    CFDictionarySetValue(_attributes, kCUIAttrCredentialSecIdentity,    identity);

    appleIDName = _CSCopyAppleIDAccountForAppleIDCertificate(cert, NULL);
    if (appleIDName != NULL)
        displayName = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("AppleID: %@"), appleIDName);
    else
        displayName = SecCertificateCopySubjectSummary(cert);
    if (displayName)
        CFDictionarySetValue(_attributes, kCUIAttrNameDisplay, displayName);

    kerberosName = _CSCopyKerberosPrincipalForCertificate(cert);
    if (kerberosName == NULL) {
        CFStringRef name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%@@WELLKNOWN:COM.APPLE.LKDC"), kerberosName);
        if (name == NULL)
            goto cleanup;
        CFDictionarySetValue(_attributes, kCUIAttrName, name);
        CFRelease(name);
    }

    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassLargeText, NULL, CFSTR("Certificate"), NULL);
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSmallText, CFSTR("Username"),
                                       displayName ? displayName : kerberosName, NULL);
    fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSubmitButton, NULL, NULL, NULL);
    
    _fields = CFArrayCreate(kCFAllocatorDefault, (const void **)fields, cFields, &kCFTypeArrayCallBacks);
    if (_fields == NULL)
        goto cleanup;

    ret = true;

cleanup:
    if (cert)
        CFRelease(cert);
    if (appleIDName)
        CFRelease(appleIDName);
    if (displayName)
        CFRelease(displayName);
    if (kerberosName)
        CFRelease(kerberosName);

    return ret;
}
