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
#include "CUIGSSItemCredentialProvider.h"
#include "CUIGSSItemCredential.h"

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
                                                 CUIUsageScenario usageScenario,
                                                 CUIUsageFlags usageFlags,
                                                 CFErrorRef *error)
{
    if (usageFlags & (kCUIUsageFlagsGeneric | kCUIUsageFlagsExcludePersistedCreds))
        return false;
    
    _controller = (CUIControllerRef)CFRetain(controller);
    _usageScenario = usageScenario;
    _usageFlags = usageFlags;
    
    return true;
}

CFArrayRef
CUIGSSItemCredentialProvider::copyMatchingCredentials(CFDictionaryRef attributes,
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
    
    items = GSSItemCopyMatching(gssItemAttributes, error);
    if (items) {
        for (CFIndex index = 0; index < CFArrayGetCount(items); index++) {
            GSSItemRef item = (GSSItemRef)CFArrayGetValueAtIndex(items, index);
            CFMutableDictionaryRef cuiAttributes = createCUIAttributesFromGSSItemAttributes(item->keys); // XXX private data
            
            if (cuiAttributes == NULL)
                continue;
            
            CFDictionarySetValue(cuiAttributes, kCUIAttrGSSItemRef, item);
            CFDictionarySetValue(cuiAttributes, kCUIAttrPersistenceFactoryID, kGSSItemCredentialProviderFactoryID);
 
            _CUIControllerEnumerateCredentialsExcepting(_controller,
                                                        0,
                                                        cuiAttributes,
                                                        kGSSItemCredentialProviderFactoryID,
                                                        ^(CUICredentialRef cred, Boolean isDefault, CFErrorRef err) {
                     CUIGSSItemCredential *itemCred;
                     CUICredentialRef credRef;
                     
                     if (cred == NULL)
                         return;

                     if (isDefault)
                         *defaultCredentialIndex = cCreds;

                     itemCred = new CUIGSSItemCredential();
                     if (!itemCred->initWithCredential(cred, this)) {
                         itemCred->Release();
                         return;
                     }
                     
                     credRef = CUICredentialCreate(CFGetAllocator(_controller), itemCred);
                     if (credRef == NULL) {
                         itemCred->Release();
                         return;
                     }
                     
                     CFArrayAppendValue(creds, credRef);
                     cCreds++;

                     CFRelease(credRef);
                     itemCred->Release();
                 });
            
            CFRelease(cuiAttributes);
        }
        
        CFRelease(items);
    }
    
    if (gssItemAttributes)
        CFRelease(gssItemAttributes);
    
    return creds;
}

Boolean
CUIGSSItemCredentialProvider::addCredentialWithAttributes(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFDictionaryRef gssItemAttributes;
    Boolean ret = false;
    struct GSSItem *item = NULL;
 
    if (error)
        *error = NULL;
    
    gssItemAttributes = createGSSItemAttributesFromCUIAttributes(attributes);
    if (gssItemAttributes == NULL)
        return false;
    
    item = GSSItemAdd(gssItemAttributes, error);
    if (item) {
        ret = true;
        CFRelease(item);
    }
    
    CFRelease(gssItemAttributes);
    
    return ret;
}

Boolean
CUIGSSItemCredentialProvider::updateCredential(CUICredentialRef credential, CFErrorRef *error)
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    CFDictionaryRef gssItemAttributes = NULL;
    Boolean ret;
    GSSItemRef item = (GSSItemRef)CFDictionaryGetValue(attributes, kCUIAttrGSSItemRef);
    
    if (item == NULL)
        return false;
    
    gssItemAttributes = createGSSItemAttributesFromCUIAttributes(attributes);
    if (gssItemAttributes == NULL)
        return false;
    
    /*
     * There's a bug in Heimdal, if the keychain item already exists it will
     * create a new one, not update it, which will fail. Whoops.
     */
    ret = GSSItemUpdate(item->keys, gssItemAttributes, error);
    
    if (gssItemAttributes)
        CFRelease(gssItemAttributes);
    
    return ret;
}

Boolean
CUIGSSItemCredentialProvider::deleteCredential(CUICredentialRef credential, CFErrorRef *error)
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(credential);
    GSSItemRef item = (GSSItemRef)CFDictionaryGetValue(attributes, kCUIAttrGSSItemRef);
    
    return GSSItemDeleteItem(item, error);
}

__attribute__((visibility("default"))) void *
CUIGSSItemCredentialProviderFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
    if (CFEqual(typeID, kCUIProviderTypeID))
        return new CUIGSSItemCredentialProvider;
    
    return NULL;
}

