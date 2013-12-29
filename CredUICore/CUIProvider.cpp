//
//  CUIProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static Boolean
CUILoadProviders(void)
{
    return false;
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
        
        if (iunk == NULL)
            continue;
        
        iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUIProviderInterfaceID), (void **)&provider);
        iunk->Release();
        
        if (!provider->initWithController(controller)) {
            provider->Release();
            continue;
        }
        
        CFArrayAppendValue(providers, provider);
    }
    
cleanup:
    if (factories)
        CFRelease(factories);
    
    return providers;
}