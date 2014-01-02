//
//  CUIProviderUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include <CredUICore/CUIAttributes.h>

#include "CUIProviderUtilities.h"

static CFStringRef kCUIPrefix = CFSTR("kCUI");
static CFStringRef kGSSPrefix = CFSTR("kGSS");

struct __CUITransformAttributesContext {
    CFMutableDictionaryRef transformedAttrs;
    Boolean toGSS;
};

static void transformAttributesCallback(const void *key, const void *value, void *_context) {
    CFMutableStringRef transformedKey = CFStringCreateMutableCopy(kCFAllocatorDefault, 0, (CFStringRef)key);
    struct __CUITransformAttributesContext *context = (struct __CUITransformAttributesContext *)_context;
    
    if (transformedKey) {
        CFStringRef src = context->toGSS ? kCUIPrefix : kGSSPrefix;
        CFStringRef dst = context->toGSS ? kGSSPrefix : kCUIPrefix;
        
        CFStringFindAndReplace(transformedKey, src, dst, CFRangeMake(0, 4), 0);
        CFDictionarySetValue(context->transformedAttrs, transformedKey, value);
        CFRelease(transformedKey);
    }
}

static CFDictionaryRef transformAttributes(CFDictionaryRef attrs, bool toGSS) {
    __CUITransformAttributesContext context;
    
    if (attrs == NULL)
        return NULL;
    
    context.transformedAttrs = CFDictionaryCreateMutable(CFGetAllocator(attrs),
                                                         CFDictionaryGetCount(attrs),
                                                         &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
    if (context.transformedAttrs == NULL)
        return NULL;
    
    context.toGSS = toGSS;
    
    CFDictionaryApplyFunction(attrs, transformAttributesCallback, (void *)&context);
    
    return context.transformedAttrs;
}

CFDictionaryRef CUICreateCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes)
{
    return transformAttributes(attributes, false);
}

CFDictionaryRef CUICreateGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes)
{
    return transformAttributes(attributes, true);
}

Boolean
CUIGetAttributeSource(CFDictionaryRef attributes)
{
    if (CFDictionaryGetValue(attributes, kCUIAttrUUID))
        return kCUIAttributeSourceGSSItem;
    else
        return kCUIAttributeSourceUser;
}

extern "C" {
    struct GSSItem;

    extern struct GSSItem *
    GSSItemAdd(CFDictionaryRef attributes, CFErrorRef *error);

    extern Boolean
    GSSItemUpdate(CFDictionaryRef query, CFDictionaryRef attributesToUpdate, CFErrorRef *error);
}

Boolean
CUIAddGSSItem(CFDictionaryRef attributes, CFErrorRef *error)
{
    CFDictionaryRef gssItemAttributes;
    Boolean ret = false;
    struct GSSItem *item = NULL;

    if (error)
        *error = NULL;

    gssItemAttributes = CUICreateGSSItemAttributesFromCUIAttributes(attributes);
    if (gssItemAttributes == NULL)
        return false;

    if (CUIGetAttributeSource(attributes) == kCUIAttributeSourceGSSItem) {
        ret = GSSItemUpdate(gssItemAttributes, gssItemAttributes, error);
    } else {
        item = GSSItemAdd(gssItemAttributes, error);
        if (item) {
            ret = true;
            CFRelease(item);
        }
    }
    
    CFRelease(gssItemAttributes);
    
    return ret;
}
