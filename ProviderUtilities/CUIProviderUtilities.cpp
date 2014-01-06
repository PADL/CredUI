//
//  CUIProviderUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"
#include "GSSItem.h"

static CFStringRef kCUIPrefix = CFSTR("kCUI");
static CFStringRef kGSSPrefix = CFSTR("kGSS");

struct _CUITransformAttributesContext {
    CFMutableDictionaryRef transformedAttrs;
    Boolean toGSS;
};

static void _CUITransformAttributesCallback(const void *key, const void *value, void *_context)
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

static CFMutableDictionaryRef _CUITransformAttributes(CFDictionaryRef attrs, bool toGSS)
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

CFMutableDictionaryRef CUICreateCUIAttributesFromGSSItemAttributes(CFDictionaryRef attributes)
{
    return _CUITransformAttributes(attributes, false);
}


CFMutableDictionaryRef CUICreateGSSItemAttributesFromCUIAttributes(CFDictionaryRef attributes)
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

CUIAttributeSource
CUIGetAttributeSource(CFDictionaryRef attributes)
{
    if (CFDictionaryGetValue(attributes, kCUIAttrGSSItemRef))
        return kCUIAttributeSourceGSSItem;
    else if (CFDictionaryGetValue(attributes, kCUIAttrSecKeychainItemRef))
        return kCUIAttributeSourceKeychain;
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
CUIGSSItemSave(CFDictionaryRef attributes, CFTypeRef targetName, CFErrorRef *error)
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

static CFNumberRef kGSSSecPasswordType;
static CFNumberRef kCUISecPasswordType;

static void
_CUIKeychainProviderUtilitiesInit(void) __attribute((__constructor__));

static void
_CUIKeychainProviderUtilitiesInit(void)
{
    uint32_t num;
    
    num = 'GSSP';
    kGSSSecPasswordType = CFNumberCreate(NULL, kCFNumberSInt32Type, &num);
    
    num = 'CUIP';
    kCUISecPasswordType = CFNumberCreate(NULL, kCFNumberSInt32Type, &num);
}

static CFStringRef
_CUIKeychainDefaultService(Boolean bCUIGeneric)
{
    static CFStringRef gss = CFSTR("GSS");
    static CFStringRef credUI = CFSTR("CredUI");
    
    return bCUIGeneric ? credUI : gss;
}

CFMutableDictionaryRef
CUICreateCUIAttributesFromKeychainAttributes(CFDictionaryRef keychainAttrs, Boolean bCUIGeneric)
{
    CFMutableDictionaryRef attributes;
    
    attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                           &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (attributes == NULL)
        return NULL;
    
    CFTypeRef accountName = CFDictionaryGetValue(keychainAttrs, kSecAttrAccount);
    if (accountName) {
        CFDictionarySetValue(attributes, bCUIGeneric ? kCUIAttrName : kCUIAttrUUID, accountName);
        if (bCUIGeneric)
            CFDictionarySetValue(attributes, kCUIAttrNameDisplay, accountName);
    } else {
        CFRelease(attributes);
        return NULL;
    }
    
    CFDictionarySetValue(attributes, kCUIAttrClass, kCUIAttrClassGeneric);
    CFDictionarySetValue(attributes, kCUIAttrCredentialExists, kCFBooleanTrue);
    CFDictionarySetValue(attributes, kCUIAttrSupportGSSCredential, bCUIGeneric ? kCFBooleanFalse : kCFBooleanTrue);
    CFDictionarySetValue(attributes, kCUIAttrNameType, kCUIAttrNameTypeGSSUsername); // XXX
    CFDictionarySetValue(attributes, kCUIAttrCredentialPassword, kCFBooleanTrue); // XXX
    
    CFTypeRef keychainItemRef = CFDictionaryGetValue(keychainAttrs, kSecValueRef);
    if (keychainItemRef)
        CFDictionarySetValue(attributes, kCUIAttrSecKeychainItemRef, keychainItemRef);
    
    CFTypeRef accessGroup = CFDictionaryGetValue(keychainAttrs, kSecAttrAccessGroup);
    if (accessGroup)
        CFDictionarySetValue(attributes, kSecAttrAccessGroup, accessGroup);
    
    return attributes;
}

CFMutableDictionaryRef
CUICreateKeychainAttributesFromCUIAttributes(CFDictionaryRef attributes,
                                             CFTypeRef targetName,
                                             Boolean *pbCUIGeneric)
{
    CFMutableDictionaryRef keychainAttrs;
    CFStringRef name;
    Boolean bCUIGeneric = (CUIGetAttributeSource(attributes) != kCUIAttributeSourceGSSItem);
    
    *pbCUIGeneric = bCUIGeneric;
    
    keychainAttrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (keychainAttrs == NULL) {
        return NULL;
    }
    
    if (targetName) {
        CFStringRef targetDisplayName = CUICopyTargetDisplayName(targetName);

        if (targetDisplayName) {
            CFDictionarySetValue(keychainAttrs, kSecAttrService, targetDisplayName);
            CFRelease(targetDisplayName);
        }
    }
    
    CFDictionarySetValue(keychainAttrs, kSecClass, kSecClassGenericPassword);
    CFDictionarySetValue(keychainAttrs, kSecAttrType, bCUIGeneric ? kCUISecPasswordType : kGSSSecPasswordType);
    if (attributes) {
        name = (CFStringRef)CFDictionaryGetValue(attributes, bCUIGeneric ? kCUIAttrName : kCUIAttrUUID);
        if (name)
            CFDictionarySetValue(keychainAttrs, kSecAttrAccount, name);
    }
    
    CFTypeRef accessGroup = CFDictionaryGetValue(keychainAttrs, kSecAttrAccessGroup);
    if (accessGroup)
        CFDictionarySetValue(keychainAttrs, kSecAttrAccessGroup, accessGroup);

    return keychainAttrs;
}

CFArrayRef
CUIKeychainCopyMatching(CFDictionaryRef attributes,
                        CFTypeRef targetName,
                        CFErrorRef *error)
{
    CFMutableDictionaryRef query = NULL;
    CFArrayRef result = NULL;
    Boolean bCUIGeneric;
    
    query = CUICreateKeychainAttributesFromCUIAttributes(attributes, targetName, &bCUIGeneric);
    if (query == NULL)
        return NULL;
    
    CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecReturnRef, kCFBooleanTrue);
    CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
    
    if (SecItemCopyMatching(query, (CFTypeRef *)&result) != errSecSuccess) {
        if (targetName) {
            CFDictionarySetValue(query, kSecAttrService, _CUIKeychainDefaultService(bCUIGeneric));
            SecItemCopyMatching(query, (CFTypeRef *)&result);
        }
    }
    
    CFRelease(query);
    
    return result;
}

Boolean
CUIKeychainSetPasswordAttr(CFMutableDictionaryRef keychainAttrs,
                           CFDictionaryRef attributes)
{
    CFTypeRef password = CFDictionaryGetValue(attributes, kCUIAttrCredentialPassword);

    if (password) {
        if (CFGetTypeID(password) == CFStringGetTypeID()) {
            CFDataRef passwordData;

            passwordData = CFStringCreateExternalRepresentation(kCFAllocatorDefault,
                                                                (CFStringRef)password, kCFStringEncodingUTF8, 0);
        
            if (passwordData == NULL)
                return false;
        
            CFDictionarySetValue(keychainAttrs, kSecValueData, passwordData);
            CFRelease(passwordData);
        } else if (CFGetTypeID(password) == CFDataGetTypeID()) {
            CFDictionarySetValue(keychainAttrs, kSecValueData, password);
        } else if (CFGetTypeID(password) != CFBooleanGetTypeID()) {
            /* Hmm, what's going on here? */
        }
    }
    
    return true;
}

Boolean
CUIKeychainSave(CFDictionaryRef attributes,
                CFTypeRef targetName,
                CFErrorRef *error)
{
    Boolean bCUIGeneric;
    CFMutableDictionaryRef keychainAttrs;
    OSStatus osret;
    CFTypeRef itemRef = NULL;

    if (targetName == NULL)
        targetName = _CUIKeychainDefaultService(true);
    
    keychainAttrs = CUICreateKeychainAttributesFromCUIAttributes(attributes, targetName, &bCUIGeneric);
    if (keychainAttrs == NULL)
        return false;
    
    CFStringRef description = CFStringCreateWithFormat(NULL, NULL, CFSTR("CredUI password for %@"),
                                                       CFDictionaryGetValue(attributes, kCUIAttrName));

    if (description == NULL) {
        CFRelease(keychainAttrs);
        return false;
    }
    
    CFDictionarySetValue(keychainAttrs, kSecAttrDescription, description);
    CFDictionarySetValue(keychainAttrs, kSecAttrLabel, description);
    CFRelease(description);

    if (!CUIKeychainSetPasswordAttr(keychainAttrs, attributes)) {
        CFRelease(keychainAttrs);
        return false;
    }
    
    osret = SecItemAdd(keychainAttrs, &itemRef);
    
    if (itemRef)
        CFRelease(itemRef);

    CFRelease(keychainAttrs);
    
    return !osret;
}

CFStringRef
CUIGetDefaultUsername(CFDictionaryRef attributes)
{
    CFStringRef defaultUsername;

    defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameDisplay);
    if (defaultUsername == NULL) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);
    }
    
    return defaultUsername;
}

Boolean
CUIShouldEnumerateForClass(CFDictionaryRef attributes, CFStringRef mechClass)
{
    CFStringRef attrClass = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass);
    
    if (attrClass == NULL)
        return TRUE;
    
    return CFEqual(attrClass, mechClass);
}

Boolean
CUIShouldEnumerateForPasswordClass(CFDictionaryRef attributes)
{
    return CUIShouldEnumerateForClass(attributes, kCUIAttrClassKerberos) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassNTLM) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassIAKerb) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassGeneric);
}
