//
//  CUIProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#include <NSSystemDirectories.h>

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

CUI_CONST_TYPE(kCUIAttrName,                           kCUIAttrName);
CUI_CONST_TYPE(kCUIAttrNameDisplay,                    kCUIAttrNameDisplay);
CUI_CONST_TYPE(kCUIAttrUUID,                           kCUIAttrUUID);
CUI_CONST_TYPE(kCUIAttrTransientExpire,                kCUIAttrTransientExpire);
CUI_CONST_TYPE(kCUIAttrTransientDefaultInClass,        kCUIAttrTransientDefaultInClass);
CUI_CONST_TYPE(kCUIAttrCredentialPassword,             kCUIAttrCredentialPassword);
CUI_CONST_TYPE(kCUIAttrCredentialStore,                kCUIAttrCredentialStore);
CUI_CONST_TYPE(kCUIAttrCredentialSecIdentity,          kCUIAttrCredentialSecIdentity);
CUI_CONST_TYPE(kCUIAttrCredentialExists,               kCUIAttrCredentialExists);
CUI_CONST_TYPE(kCUIAttrStatusPersistant,               kCUIAttrStatusPersistant);
CUI_CONST_TYPE(kCUIAttrStatusAutoAcquire,              kCUIAttrStatusAutoAcquire);
CUI_CONST_TYPE(kCUIAttrStatusTransient,                kCUIAttrStatusTransient);

CUI_CONST_TYPE(kCUIAttrStatusAutoAcquireStatus,        kCUIAttrStatusAutoAcquireStatus);

CUI_CONST_TYPE(kCUIAttrCredentialStatus,               kCUIAttrCredentialStatus);
CUI_CONST_TYPE(kCUICredentialNotFinished,              kCUICredentialNotFinished);
CUI_CONST_TYPE(kCUICredentialFinished,                 kCUICredentialFinished);
CUI_CONST_TYPE(kCUICredentialReturnCredentialFinished, kCUICredentialReturnCredentialFinished);
CUI_CONST_TYPE(kCUICredentialReturnNoCredentialFinished,kCUICredentialReturnNoCredentialFinished);

CUI_CONST_TYPE(kCUIAttrCredentialProvider,             kCUIAttrCredentialProvider);
CUI_CONST_TYPE(kCUIAttrCredentialMetaProvider,         kCUIAttrCredentialMetaProvider);

CUI_CONST_TYPE(kCUIAttrSecKeychainItemRef,             kCUIAttrSecKeychainItemRef);
CUI_CONST_TYPE(kCUIAttrGSSItemRef,                     kCUIAttrGSSItemRef);

static CFStringRef __CUIPlugInDirectory = CFSTR("CredentialProviders");
static CFStringRef __CUIPlugInBundleType = CFSTR("credprovider");

extern "C" {
extern CFArrayRef CFCopySearchPathForDirectoriesInDomains(CFIndex directory, CFIndex domainMask, Boolean expandTilde);
};

static CFMutableArrayRef plugins;

static Boolean
CUILoadProviders(void)
{
    CFArrayRef searchPaths;
    CFIndex index;
    
    plugins = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
    if (plugins == NULL)
        return false;
    
    searchPaths = CFCopySearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask | NSLocalDomainMask | NSSystemDomainMask, true);
    if (searchPaths == NULL)
        return false;
    
    for (index = 0; index < CFArrayGetCount(searchPaths); index++) {
        CFURLRef url = CFURLCreateWithFileSystemPathRelativeToBase(kCFAllocatorDefault,
                                                                   __CUIPlugInDirectory,
                                                                   kCFURLPOSIXPathStyle,
                                                                   true,
                                                                   (CFURLRef)CFArrayGetValueAtIndex(searchPaths, index));
        if (url == NULL)
            continue;
        
        CFArrayRef bundles = CFBundleCreateBundlesFromDirectory(kCFAllocatorDefault,
                                                                url,
                                                                __CUIPlugInBundleType);
        if (bundles == NULL) {
            CFRelease(url);
            continue;
        }
        
        CFArrayAppendArray(plugins, bundles, CFRangeMake(0, CFArrayGetCount(bundles)));
        CFRelease(bundles);
        CFRelease(url);
    }
    
    CFRelease(searchPaths);
    
    if (CFArrayGetCount(plugins) == 0) {
        CFRelease(plugins);
        plugins = NULL;
        return false;
    }
    
    return true;
}

void
CUIUnloadProviders(void)
{
    if (plugins) {
        CFRelease(plugins);
        plugins = NULL;
    }
}

static const void *
_CUIProviderRetain(CFAllocatorRef allocator, const void *value)
{
    CUIProvider *provider = (CUIProvider *)value;
    provider->AddRef();
    return provider;
}

static void
_CUIProviderRelease(CFAllocatorRef allocator, const void *value)
{
    CUIProvider *provider = (CUIProvider *)value;
    provider->Release();
}

static Boolean
_CUIProviderEqual(const void *value1, const void *value2)
{
    return (value1 == value2);
}

CFArrayCallBacks kCUIProviderArrayCallBacks = {
    .version = 0,
    .retain = _CUIProviderRetain,
    .release = _CUIProviderRelease,
    .copyDescription = NULL,
    .equal = _CUIProviderEqual
};

Boolean
CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller)
{
    static dispatch_once_t onceToken;
    CFArrayRef factories = NULL;
    CFIndex index;
    
    dispatch_once(&onceToken, ^{
        CUILoadProviders();
    });
    
    controller->_providers = CFArrayCreateMutable(allocator, 0, &kCUIProviderArrayCallBacks);
    if (controller->_providers == NULL)
        goto cleanup;

    controller->_factories = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
    if (controller->_factories == NULL)
        goto cleanup;

    factories = CFPlugInFindFactoriesForPlugInType(kCUIProviderTypeID);
    if (factories == NULL)
        goto cleanup;
    
    for (index = 0; index < CFArrayGetCount(factories); index++) {
        CFUUIDRef factoryID = (CFUUIDRef)CFArrayGetValueAtIndex(factories, index);
        IUnknown *iunk = (IUnknown *)CFPlugInInstanceCreate(allocator, factoryID, kCUIProviderTypeID);
        CUIProvider *provider = NULL;
        CFErrorRef error = NULL;
        
        if (iunk == NULL)
            continue;
        
        iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUIProviderInterfaceID), (void **)&provider);
        iunk->Release();
        
        if (!provider->initWithController(controller,
                                          controller->_usage,
                                          controller->_usageFlags,
                                          &error)) {
            if (error)
                CFRelease(error);
            provider->Release();
            continue;
        }
        
        CFArrayAppendValue((CFMutableArrayRef)controller->_factories, factoryID);
        CFArrayAppendValue((CFMutableArrayRef)controller->_providers, provider);
    }
    
cleanup:
    if (factories)
        CFRelease(factories);
    
    if (controller->_factories &&
        CFArrayGetCount(controller->_factories) == 0) {
        CFRelease(controller->_factories);
        controller->_factories = NULL;
    }

    if (controller->_providers &&
        CFArrayGetCount(controller->_providers) == 0) {
        CFRelease(controller->_providers);
        controller->_providers = NULL;
    }
    
    return !!controller->_providers;
}

CUI_EXPORT CFArrayRef
CUICredentialContextArrayCreate(CFAllocatorRef allocator,
                                const CUICredentialContext **contexts,
                                CFIndex numContexts)
{
    return CFArrayCreate(allocator, (const void **)contexts,
                         numContexts, &kCUICredentialContextArrayCallBacks);
}
