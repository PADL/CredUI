//
//  CUIProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#include <NSSystemDirectories.h>

#define CUI_CONST_TYPE(t,k,v) const t k = (t)(CFSTR(#v));

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrClass,                          kCUIAttrClass);
CUI_CONST_TYPE(CFStringRef, kCUIAttrClassKerberos,                  kGSSAttrClassKerberos);
CUI_CONST_TYPE(CFStringRef, kCUIAttrClassNTLM,                      kGSSAttrClassNTLM);
CUI_CONST_TYPE(CFStringRef, kCUIAttrClassIAKerb,                    kGSSAttrClassIAKerb);

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrSupportGSSCredential,           kCUIAttrSupportGSSCredential);

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrNameType,                       kCUIAttrNameType);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrNameTypeGSSExportedName,        kGSSAttrNameTypeGSSExportedName);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrNameTypeGSSUsername,            kGSSAttrNameTypeGSSUsername);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrNameTypeGSSHostBasedService,    kGSSAttrNameTypeGSSHostBasedService);

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrName,                           kCUIAttrName);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrNameDisplay,                    kCUIAttrNameDisplay);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrUUID,                           kCUIAttrUUID);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrTransientExpire,                kCUIAttrTransientExpire);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrTransientDefaultInClass,        kCUIAttrTransientDefaultInClass);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrCredentialPassword,             kCUIAttrCredentialPassword);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrCredentialStore,                kCUIAttrCredentialStore);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrCredentialSecIdentity,          kCUIAttrCredentialSecIdentity);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrCredentialExists,               kCUIAttrCredentialExists);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrStatusPersistant,               kCUIAttrStatusPersistant);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrStatusAutoAcquire,              kCUIAttrStatusAutoAcquire);
CUI_CONST_TYPE(CFTypeRef,   kCUIAttrStatusTransient,                kCUIAttrStatusTransient);

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrStatusAutoAcquireStatus,        kCUIAttrStatusAutoAcquireStatus);

CUI_CONST_TYPE(CFTypeRef,   kCUIAttrCredentialStatus,               kCUIAttrCredentialStatus);
CUI_CONST_TYPE(CFStringRef, kCUICredentialNotFinished,              kCUICredentialNotFinished);
CUI_CONST_TYPE(CFStringRef, kCUICredentialFinished,                 kCUICredentialFinished);
CUI_CONST_TYPE(CFStringRef, kCUICredentialReturnCredentialFinished, kCUICredentialReturnCredentialFinished);
CUI_CONST_TYPE(CFStringRef, kCUICredentialReturnNoCredentialFinished,kCUICredentialReturnNoCredentialFinished);

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

CFArrayRef
CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller)
{
    static dispatch_once_t onceToken;
    CFMutableArrayRef providers = NULL;
    CFArrayRef factories = NULL;
    CFIndex index;
    
    dispatch_once(&onceToken, ^{
        CUILoadProviders();
    });
    
    providers = CFArrayCreateMutable(allocator, 0, &kCUIProviderArrayCallBacks);
    if (providers == NULL)
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
        
        if (!provider->initWithController(controller, &error)) {
            if (error)
                CFRelease(error);
            provider->Release();
            continue;
        }
        
        CFArrayAppendValue(providers, provider);
    }
    
cleanup:
    if (factories)
        CFRelease(factories);
    
    if (providers && CFArrayGetCount(providers) == 0) {
        CFRelease(providers);
        providers = NULL;
    }
    
    return providers;
}
