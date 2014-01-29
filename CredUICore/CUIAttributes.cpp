//
//  CUIAttributes.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#define CUI_CONST_TYPE(k,v) CUI_EXPORT const CFStringRef k = CFSTR(#v);

CUI_CONST_TYPE(kCUIAttrClass,                          kCUIAttrClass);
CUI_CONST_TYPE(kCUIAttrClassKerberos,                  kGSSAttrClassKerberos);
CUI_CONST_TYPE(kCUIAttrClassNTLM,                      kGSSAttrClassNTLM);
CUI_CONST_TYPE(kCUIAttrClassIAKerb,                    kGSSAttrClassIAKerb);
CUI_CONST_TYPE(kCUIAttrClassGeneric,                   kCUIAttrClassGeneric);

CUI_CONST_TYPE(kCUIAttrSupportGSSCredential,           kCUIAttrSupportGSSCredential);

CUI_CONST_TYPE(kCUIAttrNameType,                       kCUIAttrNameType);
CUI_CONST_TYPE(kCUIAttrNameTypeGSSExportedName,        kGSSAttrNameTypeGSSExportedName);
CUI_CONST_TYPE(kCUIAttrNameTypeGSSUsername,            kGSSAttrNameTypeGSSUsername);
CUI_CONST_TYPE(kCUIAttrNameTypeGSSHostBasedService,    kGSSAttrNameTypeGSSHostBasedService);
CUI_CONST_TYPE(kCUIAttrNameTypePosixName,              kCUIAttrNameTypePosixName);

CUI_CONST_TYPE(kCUIAttrName,                           kCUIAttrName);
CUI_CONST_TYPE(kCUIAttrNameDisplay,                    kCUIAttrNameDisplay);
CUI_CONST_TYPE(kCUIAttrUUID,                           kCUIAttrUUID);
CUI_CONST_TYPE(kCUIAttrTransientExpire,                kCUIAttrTransientExpire);
CUI_CONST_TYPE(kCUIAttrTransientDefaultInClass,        kCUIAttrTransientDefaultInClass);
CUI_CONST_TYPE(kCUIAttrCredentialPassword,             kCUIAttrCredentialPassword);
CUI_CONST_TYPE(kCUIAttrCredentialStore,                kCUIAttrCredentialStore);
CUI_CONST_TYPE(kCUIAttrCredentialSecIdentity,          kCUIAttrCredentialSecIdentity);
CUI_CONST_TYPE(kCUIAttrCredentialSecCertificate,       kCUIAttrCredentialSecCertificate);
CUI_CONST_TYPE(kCUIAttrCredentialExists,               kCUIAttrCredentialExists);
CUI_CONST_TYPE(kCUIAttrStatusPersistant,               kCUIAttrStatusPersistant);
CUI_CONST_TYPE(kCUIAttrStatusAutoAcquire,              kCUIAttrStatusAutoAcquire);
CUI_CONST_TYPE(kCUIAttrStatusTransient,                kCUIAttrStatusTransient);

CUI_CONST_TYPE(kCUIAttrStatusAutoAcquireStatus,        kCUIAttrStatusAutoAcquireStatus);

CUI_CONST_TYPE(kCUIAttrCredentialError,                kCUIAttrCredentialError);

CUI_CONST_TYPE(kCUIAttrCredentialStatus,               kCUIAttrCredentialStatus);
CUI_CONST_TYPE(kCUICredentialNotFinished,              kCUICredentialNotFinished);
CUI_CONST_TYPE(kCUICredentialFinished,                 kCUICredentialFinished);
CUI_CONST_TYPE(kCUICredentialReturnCredentialFinished, kCUICredentialReturnCredentialFinished);
CUI_CONST_TYPE(kCUICredentialReturnNoCredentialFinished,kCUICredentialReturnNoCredentialFinished);
CUI_CONST_TYPE(kCUICredentialAutoSubmitCredentialFinished, kCUICredentialAutoSubmitCredentialFinished);

CUI_CONST_TYPE(kCUIAttrCredentialProvider,             kCUIAttrCredentialProvider);
CUI_CONST_TYPE(kCUIAttrCredentialMetaProvider,         kCUIAttrCredentialMetaProvider);

CUI_CONST_TYPE(kCUIAttrSecKeychainItem,                kCUIAttrSecKeychainItem);
CUI_CONST_TYPE(kCUIAttrGSSItem,                        kCUIAttrGSSItem);
CUI_CONST_TYPE(kCUIAttrCSIdentity,                     kCUIAttrCSIdentity);
CUI_CONST_TYPE(kCUIAttrGSSCredential,                  kCUIAttrGSSCredential);

CUI_CONST_TYPE(kCUIAttrPersistenceFactoryID,           kCUIAttrPersistenceFactoryID);
CUI_CONST_TYPE(kCUIAttrIdentityFactoryID,              kCUIAttrIdentityFactoryID);
CUI_CONST_TYPE(kCUIAttrProviderFactoryID,              kCUIAttrProviderFactoryID);

/* XXX These unwieldy names are to support the kCUIAttrCredential -> kGSSIC transform */
CUI_CONST_TYPE(kCUIAttrCredentialVerifyCredential,     kCUIAttrCredentialVerifyCredential)
CUI_CONST_TYPE(kCUIAttrCredentialLKDCHostname,         kCUIAttrCredentialLKDCHostname)
CUI_CONST_TYPE(kCUIAttrCredentialKerberosCacheName,    kCUIAttrCredentialKerberosCacheName)
CUI_CONST_TYPE(kCUIAttrCredentialAppIdentifierACL,     kCUIAttrCredentialAppIdentifierACL)

CUI_CONST_TYPE(kCUIAttrImageData,                      kCUIAttrImageData)
CUI_CONST_TYPE(kCUIAttrImageDataType,                  kCUIAttrImageDataType)
