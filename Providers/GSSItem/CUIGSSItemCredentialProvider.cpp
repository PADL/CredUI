//
//  GSSItemCredentialProvider.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include <libkern/OSAtomic.h>

#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>

#include "GSSItem.h"
#include "CUIProviderUtilities.h"
#include "CUIPersistedCredential.h"
#include "CUIGSSItemCredentialProvider.h"
#include "CUIGSSUtilities.h"

extern "C" {
    void *CUIGSSItemCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID);
};

static CFStringRef kCUIPrefix = CFSTR("kCUI");
static CFStringRef kGSSPrefix = CFSTR("kGSS");

struct _CUITransformAttributesContext {
    CFMutableDictionaryRef transformedAttrs;
    Boolean toGSS;
};

static void
_CUITransformAttributesCallback(const void *key, const void *value, void *_context)
{
    CFMutableStringRef transformedKey = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, (CFStringRef)key);
    struct _CUITransformAttributesContext *context = (struct _CUITransformAttributesContext *)_context;
    
    if (transformedKey) {
        CFStringRef src = context->toGSS ? kCUIPrefix : kGSSPrefix;
        CFStringRef dst = context->toGSS ? kGSSPrefix : kCUIPrefix;
        
        CFStringFindAndReplace(transformedKey, src, dst, CFRangeMake(0, 4), 0);
        CFDictionarySetValue(context->transformedAttrs, transformedKey, value);
        CFRelease(transformedKey);
    }
}

static CFMutableDictionaryRef
_CUITransformAttributes(CFDictionaryRef attrs, bool toGSS)
{
    _CUITransformAttributesContext context;
    
    if (attrs == NULL)
        return NULL;
    
    context.transformedAttrs = CFDictionaryCreateMutable(CFGetAllocator(attrs),
                                                         CFDictionaryGetCount(attrs),
                                                         &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
    if (context.transformedAttrs == NULL)
        return NULL;
    
    context.toGSS = toGSS;
    
    CFDictionaryApplyFunction(attrs, _CUITransformAttributesCallback, (void *)&context);
    
    return context.transformedAttrs;
}

CFMutableDictionaryRef
CUIGSSItemCredentialProvider::createCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes)
{
    return _CUITransformAttributes(attributes, false);
}

CFMutableDictionaryRef
CUIGSSItemCredentialProvider::createGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes)
{
    CFMutableDictionaryRef gssAttrs = _CUITransformAttributes(attributes, true);
    
    if (!CFDictionaryGetCount(attributes))
        return gssAttrs; /* query only */
    
    /*
     * Can't commit a placeholder password; that just means that it hasn't
     * changed.
     */
    CFTypeRef password = CFDictionaryGetValue(gssAttrs, kGSSAttrCredentialPassword);
    if (password) {
        if (CFEqual(password, kCFBooleanTrue))
            CFDictionaryRemoveValue(gssAttrs, kGSSAttrCredentialPassword);
        else
            CFDictionarySetValue(gssAttrs, kGSSAttrCredentialStore, kCFBooleanTrue);
    }
    
    CFTypeRef itemClass = CFDictionaryGetValue(gssAttrs, kGSSAttrClass);
    if (itemClass == NULL)
        CFDictionarySetValue(gssAttrs, kGSSAttrClass, kGSSAttrClassKerberos);
    
    return gssAttrs;
}

Boolean
CUIGSSItemCredentialProvider::initWithController(CUIControllerRef controller,
                                                 CFErrorRef *error)
{
    if (CUIControllerGetUsageScenario(controller) != kCUIUsageScenarioNetwork)
        return false;

    if (CUIControllerGetUsageFlags(controller) & (kCUIUsageFlagsGeneric | kCUIUsageFlagsExcludePersistedCreds))
        return false;
    
    _controller = (CUIControllerRef)CFRetain(controller);
    
    return true;
}

CFArrayRef
CUIGSSItemCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
                                                      CUIUsageFlags usageFlags,
                                                      CFIndex *defaultCredentialIndex,
                                                      CFErrorRef *error)
{
    CFDictionaryRef gssItemAttributes = NULL;
    CFArrayRef items;
    __block CFIndex cCreds = 0;
    CFMutableArrayRef creds = CFArrayCreateMutable(CFGetAllocator(_controller),
                                                   0,
                                                   &kCFTypeArrayCallBacks);
    
    if (attributes)
        gssItemAttributes = createGSSItemAttributesFromCUIAttributes(attributes);
    
    usageFlags |= kCUIUsageFlagsKeepUsername | kCUIUsageFlagsExcludePersistedCreds | kCUIUsageFlagsExcludeTransientCreds;

    items = GSSItemCopyMatching(gssItemAttributes, error);
    if (items) {
        for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
            GSSItemRef item = (GSSItemRef)CFArrayGetValueAtIndex(items, index);
            CFMutableDictionaryRef cuiAttributes = createCUIAttributesFromGSSItemAttributes(item->keys); // XXX private data
            
            if (cuiAttributes == NULL)
                continue;
            
            CFDictionarySetValue(cuiAttributes, kCUIAttrGSSItem, item);
            CFDictionarySetValue(cuiAttributes, kCUIAttrPersistenceFactoryID, kGSSItemCredentialProviderFactoryID);
 
            _CUIControllerEnumerateCredentialsWithFlags(_controller,
                                                        usageFlags,
                                                        cuiAttributes,
                                                        ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
                        CUICredentialRef wrappedCred;
                        
                        if (cred == NULL)
                            return;
                        
                        if (isDefault)
                            *defaultCredentialIndex = cCreds;
                        
                        wrappedCred = CUIPersistedCredentialCreate(this, cred);
                        if (wrappedCred) {
                            CFArrayAppendValue(creds, wrappedCred);
                            cCreds++;
                            
                            CFRelease(wrappedCred);
                        }
                 });
            
            CFRelease(cuiAttributes);
        }
        
        CFRelease(items);
    }
    
    if (gssItemAttributes)
        CFRelease(gssItemAttributes);
    
    return creds;
}

void
CUIGSSItemCredentialProvider::addCredentialWithAttributes(CFDictionaryRef attributes, void (^completionHandler)(CFErrorRef))
{
    CFDictionaryRef gssItemAttributes;
    GSSItemRef item = NULL;
    CFErrorRef error = NULL;
    
    gssItemAttributes = createGSSItemAttributesFromCUIAttributes(attributes);
    if (gssItemAttributes == NULL) {
        CUIGSSErrorComplete(completionHandler, GSS_S_NO_CRED);
        return;
    }
    
    item = GSSItemAdd(gssItemAttributes, &error);
    completionHandler(error);
    if (item)
        CFRelease(item);
    if (error)
        CFRelease(error);
    if (gssItemAttributes)
        CFRelease(gssItemAttributes);
}

void
CUIGSSItemCredentialProvider::updateCredential(CUICredentialRef credential, void (^completionHandler)(CFErrorRef))
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    CFDictionaryRef gssItemAttributes = NULL;
    GSSItemRef item = (GSSItemRef)CFDictionaryGetValue(attributes, kCUIAttrGSSItem);
    CFErrorRef error = NULL;
    
    if (item == NULL) {
        CUIGSSErrorComplete(completionHandler, GSS_S_NO_CRED);
        return;
    }
    
    gssItemAttributes = createGSSItemAttributesFromCUIAttributes(attributes);
    if (gssItemAttributes == NULL) {
        CUIGSSErrorComplete(completionHandler, GSS_S_NO_CRED);
        return;
    }
    
    /*
     * There's a bug in Heimdal, if the keychain item already exists it will
     * create a new one, not update it, which will fail. Whoops.
     */
    GSSItemUpdate(item->keys, gssItemAttributes, &error);
    completionHandler(error);
    
    if (error)
        CFRelease(error);
    if (gssItemAttributes)
        CFRelease(gssItemAttributes);
}

void
CUIGSSItemCredentialProvider::deleteCredential(CUICredentialRef credential, void (^completionHandler)(CFErrorRef))
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    GSSItemRef item = (GSSItemRef)CFDictionaryGetValue(attributes, kCUIAttrGSSItem);
    CFErrorRef error = NULL;
    
    GSSItemDeleteItem(item, &error);
    completionHandler(error);
    if (error)
        CFRelease(error);
}

__attribute__((visibility("default"))) void *
CUIGSSItemCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIGSSItemCredentialProvider;
    
    return NULL;
}

