/*
 * Copyright (c) 2011 Kungliga Tekniska Högskolan
 * (Royal Institute of Technology, Stockholm, Sweden).
 * All rights reserved.
 *
 * Portions Copyright (c) 2011 Apple Inc. All rights reserved.
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

#import <CoreFoundation/CoreFoundation.h>
#import <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif
    
/*
 * Type is any of the kCUIAttrTypeNNN credential types below, type are
 * strings
 */
extern const CFStringRef kCUIAttrClass;
extern const CFStringRef kCUIAttrClassKerberos;
extern const CFStringRef kCUIAttrClassNTLM;
extern const CFStringRef kCUIAttrClassIAKerb;
extern const CFStringRef kCUIAttrClassGeneric;
    
/*
 * Item supports acquiring a gss_cred_id_t with GSSItemOperation
 */
extern const CFStringRef kCUIAttrSupportGSSCredential;

/*
 * kCUIAttrNameGSSExportedName, kCUIAttrNameGSSUsername,
 * kCUIAttrNameGSSServiceBasedHostname, can set and will be returned
 *
 * kCUIAttrNameDisplay can only be returned, constructed from the
 * other name types after creation.
 */
extern const CFStringRef kCUIAttrNameType;
extern const CFStringRef kCUIAttrNameTypeGSSExportedName; /* CFDataRef */
extern const CFStringRef kCUIAttrNameTypeGSSUsername; /* CFStringRef */
extern const CFStringRef kCUIAttrNameTypeGSSHostBasedService; /* CFStringRef */

extern const CFStringRef kCUIAttrName;

/* name suiteable to display to user */
extern const CFStringRef kCUIAttrNameDisplay; /* CFStringRef */

/*
 * Unique UUID for this entry
 */
extern const CFStringRef kCUIAttrUUID; /* CFUUIDRef */


/*
 * If the item is a transient credential it can have associated
 * expiration time.
 */
extern const CFStringRef kCUIAttrTransientExpire;	/* CFDateRef */
extern const CFStringRef kCUIAttrTransientDefaultInClass; /* CFBooleanRef */
/*
 * Credential to use to use when acquiring with with
 * GSSItemOperation(kCUIOperationAcquire) or when dealing with a
 * persistant credential.
 *
 * The credentials is not exportable and will always show up as
 * the cfobject kCUIAttrCredentialExists when queried.
 */

extern const CFStringRef kCUIAttrCredentialPassword; /* CFStringRef */
extern const CFStringRef kCUIAttrCredentialStore; /* CFBooleanRef */
extern const CFStringRef kCUIAttrCredentialSecIdentity; /* SecIdentityRef */
extern const CFStringRef kCUIAttrCredentialSecCertificate; /* SecCertificateRef */
extern const CFStringRef kCUIAttrCredentialExists;

/*
 * Status of a credentials
 */

extern const CFStringRef kCUIAttrStatusPersistant;
extern const CFStringRef kCUIAttrStatusAutoAcquire;
extern const CFStringRef kCUIAttrStatusAutoAcquireStatus;
extern const CFStringRef kCUIAttrStatusTransient;

extern const CFStringRef kCUIAttrCredentialProvider;
extern const CFStringRef kCUIAttrCredentialMetaProvider;

extern const CFStringRef kCUIAttrGSSItemRef;
extern const CFStringRef kCUIAttrSecKeychainItemRef;
extern const CFStringRef kCUIAttrPersistenceFactoryID; /* CFUUIDRef */
extern const CFStringRef kCUIAttrProviderFactoryID; /* CFUUIDRef */

/*
 * For gss_aapl_initial_cred() only
 */
extern const CFStringRef kCUIAttrCredentialVerifyCredential;
extern const CFStringRef kCUIAttrCredentialLKDCHostname;
extern const CFStringRef kCUIAttrCredentialKerberosCacheName;
extern const CFStringRef kCUIAttrCredentialAppIdentifierACL;

#ifdef __cplusplus
}
#endif
