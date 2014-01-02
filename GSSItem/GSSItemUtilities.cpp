//
//  GSSItemUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>

#include "GSSItemUtilities.h"

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

CFDictionaryRef GSSItemUtilities::createCUIAttributes(CFDictionaryRef attributes) {
    return transformAttributes(attributes, false);
}

CFDictionaryRef GSSItemUtilities::createGSSItemAttributes(CFDictionaryRef attributes) {
    return transformAttributes(attributes, true);
}
