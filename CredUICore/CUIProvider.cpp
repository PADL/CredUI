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

static void
_CUILibraryFinalize(void) __attribute__((__destructor__));

static void
_CUICopyInfoDictionaryForFactoryCallback(const void *key, const void *value, void *context)
{
    if (CFGetTypeID(key) == CFStringGetTypeID()) {
        CFUUIDRef uuid = CFUUIDCreateFromString(kCFAllocatorDefault, (CFStringRef)key);
        if (uuid) {
            CFArrayAppendValue((CFMutableArrayRef)context, uuid);
            CFRelease(uuid);
        }
    }
}

static CFDictionaryRef
_CUICopyInfoDictionaryForFactory(CFUUIDRef factory)
{
    CFIndex index;
    CFDictionaryRef foundInfo = NULL;
    
    if (plugins == NULL)
        return NULL;
    
    for (index = 0; index < CFArrayGetCount(plugins); index++) {
        CFDictionaryRef info = (CFDictionaryRef)CFBundleGetInfoDictionary((CFBundleRef)CFArrayGetValueAtIndex(plugins, index));
        
        if (info == NULL)
            continue;
        
        CFDictionaryRef factoryDict = (CFDictionaryRef)CFDictionaryGetValue(info, kCFPlugInFactoriesKey);
        if (factoryDict && CFGetTypeID(factoryDict) == CFDictionaryGetTypeID()) {
            CFMutableArrayRef uuids = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
            CFDictionaryApplyFunction(factoryDict, _CUICopyInfoDictionaryForFactoryCallback, uuids);

            if (CFArrayContainsValue(uuids, CFRangeMake(0, CFArrayGetCount(uuids)), factory))
                foundInfo = (CFDictionaryRef)CFRetain(info);

            CFRelease(uuids);

            if (foundInfo)
                break;
        }
    }

    return foundInfo;
}

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

static void
_CUILibraryFinalize(void)
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

static Boolean
_CUITestDictionaryValueP(CFDictionaryRef dict, CFStringRef key)
{
    CFTypeRef cf = CFDictionaryGetValue(dict, key);

    return (cf && CFGetTypeID(cf) == CFBooleanGetTypeID() && CFBooleanGetValue((CFBooleanRef)cf));
}

Boolean
_CUIProvidersCreate(CFAllocatorRef allocator, CUIControllerRef controller)
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

    controller->_providerAttributes = CFArrayCreateMutable(allocator, 0, &kCFTypeArrayCallBacks);
    if (controller->_providerAttributes == NULL)
        goto cleanup;

    factories = CFPlugInFindFactoriesForPlugInType(kCUIProviderTypeID);
    if (factories == NULL)
        goto cleanup;
    
    for (index = 0; index < CFArrayGetCount(factories); index++) {
        CFUUIDRef factoryID = (CFUUIDRef)CFArrayGetValueAtIndex(factories, index);
        IUnknown *iunk = (IUnknown *)CFPlugInInstanceCreate(allocator, factoryID, kCUIProviderTypeID);
        CUIProvider *provider = NULL;
        CFErrorRef error = NULL;
        CFMutableDictionaryRef providerAttributes = NULL;
        
        if (iunk == NULL)
            continue;
        
        iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUIProviderInterfaceID), (void **)&provider);
        iunk->Release();
    
        providerAttributes = CFDictionaryCreateMutable(allocator,
                                                       0,
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
        if (providerAttributes == NULL) {
            provider->Release();
            goto cleanup;
        }
        
        if (!provider->initWithController(controller, &error)) {
            if (error)
                CFRelease(error);
            CFRelease(providerAttributes);
            provider->Release();
            continue;
        }
        
        CFDictionaryRef infoDict = _CUICopyInfoDictionaryForFactory(factoryID);
        if (infoDict == NULL) {
            CFRelease(providerAttributes);
            provider->Release();
            continue;
        }
        
        if (_CUITestDictionaryValueP(infoDict, CFSTR("CUIIsPersistenceProvider")))
            CFDictionarySetValue(providerAttributes, kCUIAttrPersistenceFactoryID, factoryID);
        else if (_CUITestDictionaryValueP(infoDict, CFSTR("CUIIsIdentityProvider")))
            CFDictionarySetValue(providerAttributes, kCUIAttrIdentityFactoryID, factoryID);
        else     
            CFDictionarySetValue(providerAttributes, kCUIAttrProviderFactoryID, factoryID);

        CFTypeRef supportedClasses = CFDictionaryGetValue(infoDict, CFSTR("CUISupportedClasses"));
        if (supportedClasses && CFGetTypeID(supportedClasses) == CFArrayGetTypeID())
            CFDictionarySetValue(providerAttributes, kCUIAttrClass, supportedClasses);
        
        CFArrayAppendValue((CFMutableArrayRef)controller->_providerAttributes, providerAttributes);
        CFArrayAppendValue((CFMutableArrayRef)controller->_providers, provider);
        
        CFRelease(providerAttributes);
        CFRelease(infoDict);
    }
    
cleanup:
    if (factories)
        CFRelease(factories);
    
    if (controller->_providerAttributes &&
        CFArrayGetCount(controller->_providerAttributes) == 0) {
        CFRelease(controller->_providerAttributes);
        controller->_providerAttributes = NULL;
    }

    if (controller->_providers &&
        CFArrayGetCount(controller->_providers) == 0) {
        CFRelease(controller->_providers);
        controller->_providers = NULL;
    }
    
    return !!controller->_providers;
}

static IUnknown *
_CUIControllerFindProviderByFactoryID(CUIControllerRef controller, CFUUIDRef desiredFactoryID, Boolean persistence)
{
    CFIndex index;
    
    for (index = 0; index < CFArrayGetCount(controller->_providerAttributes); index++) {
        CFDictionaryRef attributes = (CFDictionaryRef)CFArrayGetValueAtIndex(controller->_providerAttributes, index);
        CFUUIDRef factoryID;
        
         factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes,
                                                     persistence ? kCUIAttrPersistenceFactoryID : kCUIAttrProviderFactoryID);
         if (factoryID && CFEqual(factoryID, desiredFactoryID))
            return (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);
    }

    return NULL;
}

CUI_EXPORT CUIProvider *
CUIControllerFindProviderByFactoryID(CUIControllerRef controller, CFUUIDRef factoryID)
{
    return (CUIProvider *)_CUIControllerFindProviderByFactoryID(controller, factoryID, false);
}

CUI_EXPORT CUICredentialPersistence *
CUIControllerCreatePersistenceForFactoryID(CUIControllerRef controller, CFUUIDRef factoryID)
{
    IUnknown *iunk = _CUIControllerFindProviderByFactoryID(controller, factoryID, true);
    CUICredentialPersistence *persistence = NULL;
    
    if (iunk == NULL)
        return NULL;
    
    iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUIPersistenceInterfaceID), (void **)&persistence);
    iunk->Release();
    
    return persistence;
}
