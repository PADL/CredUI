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
 * Type is any of the kGSSAttrTypeNNN credential types below, type are
 * strings
 */
extern const CFTypeRef kGSSAttrClass
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

extern const CFStringRef kGSSAttrClassKerberos
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFStringRef kGSSAttrClassNTLM
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFStringRef kGSSAttrClassIAKerb
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/*
 * Item supports acquiring a gss_cred_id_t with GSSItemOperation
 */
extern const CFTypeRef kGSSAttrSupportGSSCredential
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/*
 * kGSSAttrNameGSSExportedName, kGSSAttrNameGSSUsername,
 * kGSSAttrNameGSSServiceBasedHostname, can set and will be returned
 *
 * kGSSAttrNameDisplay can only be returned, constructed from the
 * other name types after creation.
 */
extern const CFTypeRef kGSSAttrNameType
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrNameTypeGSSExportedName /* CFDataRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrNameTypeGSSUsername /* CFStringRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrNameTypeGSSHostBasedService /* CFStringRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

extern const CFTypeRef kGSSAttrName
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/* name suiteable to display to user */
extern const CFTypeRef kGSSAttrNameDisplay /* CFStringRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/*
 * Unique UUID for this entry
 */
extern const CFTypeRef kGSSAttrUUID /* CFUUIDRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);


/*
 * If the item is a transient credential it can have associated
 * expiration time.
 */
extern const CFTypeRef kGSSAttrTransientExpire	/* CFDateRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrTransientDefaultInClass /* CFBooleanRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
/*
 * Credential to use to use when acquiring with with
 * GSSItemOperation(kGSSOperationAcquire) or when dealing with a
 * persistant credential.
 *
 * The credentials is not exportable and will always show up as
 * the cfobject kGSSAttrCredentialExists when queried.
 */

extern const CFTypeRef kGSSAttrCredentialPassword /* CFStringRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrCredentialStore /* CFBooleanRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrCredentialSecIdentity /* SecIdentityRef */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrCredentialExists
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/*
 * Status of a credentials
 */

extern const CFTypeRef kGSSAttrStatusPersistant
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrStatusAutoAcquire
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrStatusAutoAcquireStatus
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSAttrStatusTransient
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

#ifdef __cplusplus
}
#endif