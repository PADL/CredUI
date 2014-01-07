//
//  CUIProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

#include <NSSystemDirectories.h>

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

CUI_EXPORT CUIProvider *
__CUIControllerFindProviderByFactoryID(CUIControllerRef controller, CFUUIDRef factoryID)
{
    CFIndex index = CFArrayGetFirstIndexOfValue(controller->_factories,
                                                CFRangeMake(0, CFArrayGetCount(controller->_factories)),
                                                (CFTypeRef)factoryID);
    
    if (index == kCFNotFound)
        return NULL;
    
    return (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);
}

CUI_EXPORT CUICredentialPersistence *
__CUIControllerCreatePersistenceForFactoryID(CUIControllerRef controller, CFUUIDRef factoryID)
{
    IUnknown *iunk = (IUnknown *)__CUIControllerFindProviderByFactoryID(controller, factoryID);
    CUICredentialPersistence *persistence = NULL;
    
    if (iunk == NULL)
        return NULL;
    
    iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUIPersistenceInterfaceID), (void **)&persistence);
    iunk->Release();
    
    return persistence;
}


