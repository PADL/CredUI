/*
 * Copyright (c) 2011 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Portions Copyright (c) 2011 Apple Inc. All rights reserved.
 * Portions Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of KTH nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CredUICore_CUIAttributes_h
#define CredUICore_CUIAttributes_h

#include <CredUICore/CUIBase.h>

#ifdef __cplusplus
extern "C" {
#endif
    
/*
 * Type is any of the kCUIAttrTypeNNN credential types below, type are
 * strings
 */
CUI_EXPORT const CFStringRef kCUIAttrClass;
CUI_EXPORT const CFStringRef kCUIAttrClassKerberos;
CUI_EXPORT const CFStringRef kCUIAttrClassNTLM;
CUI_EXPORT const CFStringRef kCUIAttrClassIAKerb;
CUI_EXPORT const CFStringRef kCUIAttrClassGeneric;
    
/*
 * Item supports acquiring a gss_cred_id_t with GSSItemOperation
 */
CUI_EXPORT const CFStringRef kCUIAttrSupportGSSCredential;

/*
 * kCUIAttrNameGSSExportedName, kCUIAttrNameGSSUsername,
 * kCUIAttrNameGSSServiceBasedHostname, can set and will be returned
 *
 * kCUIAttrNameDisplay can only be returned, constructed from the
 * other name types after creation.
 */
CUI_EXPORT const CFStringRef kCUIAttrNameType;
CUI_EXPORT const CFStringRef kCUIAttrNameTypeGSSExportedName; /* CFDataRef */
CUI_EXPORT const CFStringRef kCUIAttrNameTypeGSSUsername; /* CFStringRef */
CUI_EXPORT const CFStringRef kCUIAttrNameTypeGSSHostBasedService; /* CFStringRef */

CUI_EXPORT const CFStringRef kCUIAttrName;

/* name suiteable to display to user */
CUI_EXPORT const CFStringRef kCUIAttrNameDisplay; /* CFStringRef */

/*
 * Unique UUID for this entry
 */
CUI_EXPORT const CFStringRef kCUIAttrUUID; /* CFUUIDRef */


/*
 * If the item is a transient credential it can have associated
 * expiration time.
 */
CUI_EXPORT const CFStringRef kCUIAttrTransientExpire;	/* CFDateRef */
CUI_EXPORT const CFStringRef kCUIAttrTransientDefaultInClass; /* CFBooleanRef */
/*
 * Credential to use to use when acquiring with with
 * GSSItemOperation(kCUIOperationAcquire) or when dealing with a
 * persistant credential.
 *
 * The credentials is not exportable and will always show up as
 * the cfobject kCUIAttrCredentialExists when queried.
 */

CUI_EXPORT const CFStringRef kCUIAttrCredentialPassword; /* CFStringRef */
CUI_EXPORT const CFStringRef kCUIAttrCredentialStore; /* CFBooleanRef */
CUI_EXPORT const CFStringRef kCUIAttrCredentialSecIdentity; /* SecIdentityRef */
CUI_EXPORT const CFStringRef kCUIAttrCredentialSecCertificate; /* SecCertificateRef */
CUI_EXPORT const CFStringRef kCUIAttrCredentialExists;

/*
 * Status of a credentials
 */

CUI_EXPORT const CFStringRef kCUIAttrStatusPersistant;
CUI_EXPORT const CFStringRef kCUIAttrStatusAutoAcquire;
CUI_EXPORT const CFStringRef kCUIAttrStatusAutoAcquireStatus;
CUI_EXPORT const CFStringRef kCUIAttrStatusTransient;

CUI_EXPORT const CFStringRef kCUIAttrCredentialProvider;
CUI_EXPORT const CFStringRef kCUIAttrCredentialMetaProvider;

CUI_EXPORT const CFStringRef kCUIAttrGSSItemRef;
CUI_EXPORT const CFStringRef kCUIAttrSecKeychainItemRef;
CUI_EXPORT const CFStringRef kCUIAttrPersistenceFactoryID; /* CFUUIDRef */
CUI_EXPORT const CFStringRef kCUIAttrProviderFactoryID; /* CFUUIDRef */

/*
 * For gss_aapl_initial_cred() only
 */
CUI_EXPORT const CFStringRef kCUIAttrCredentialVerifyCredential;
CUI_EXPORT const CFStringRef kCUIAttrCredentialLKDCHostname;
CUI_EXPORT const CFStringRef kCUIAttrCredentialKerberosCacheName;
CUI_EXPORT const CFStringRef kCUIAttrCredentialAppIdentifierACL;

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIAttributes_h */
