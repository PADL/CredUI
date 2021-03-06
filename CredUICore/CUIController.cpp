//
//  CUIController.cpp
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static CFTypeID _CUIControllerTypeID = _kCFRuntimeNotATypeID;

void
_CUIControllerShowProviders(CUIControllerRef controller);

static inline void
_CUISetter(CFTypeRef &dst, CFTypeRef src)
{
    if (dst != src) {
        if (dst) {
            CFRelease(dst);
            dst = NULL;
        }
        if (src) {
            dst = CFRetain(src);
        }
    }
}

static void _CUIControllerDeallocate(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    if (controller->_providerAttributes)
        CFRelease(controller->_providerAttributes);
    if (controller->_providers)
        CFRelease(controller->_providers);
    if (controller->_uiContext.parentWindow)
        CFRelease(controller->_uiContext.parentWindow);    
    if (controller->_uiContext.messageText)
        CFRelease(controller->_uiContext.messageText);    
    if (controller->_uiContext.titleText)
        CFRelease(controller->_uiContext.titleText);    
    if (controller->_attributes)
        CFRelease(controller->_attributes);
    if (controller->_authError)
        CFRelease(controller->_authError);
    if (controller->_context)
        CFRelease(controller->_context);
    if (controller->_targetName)
        CFRelease(controller->_targetName);
}

static CFStringRef _CUIControllerGetUsageScenarioString(CUIUsageScenario usageScenario)
{
    CFStringRef string;

    switch (usageScenario) {
        case kCUIUsageScenarioLogin:
            string = CFSTR("Login");
            break;
        case kCUIUsageScenarioUnlock:
            string = CFSTR("Unlock");
            break;
        case kCUIUsageScenarioChangePassword:
            string = CFSTR("ChangePassword");
            break;
        case kCUIUsageScenarioNetwork:
            string = CFSTR("Network");
            break;
        case kCUIUsageScenarioSSO:
            string = CFSTR("SSO");
            break;
        default:
            string = NULL;
            break;
    }

    return string;
}

static CFStringRef _CUIControllerCopyDescription(CFTypeRef cf)
{
    CUIControllerRef controller = (CUIControllerRef)cf;
    
    return CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                    CFSTR("<CUIController %p>{usage = %@, usageFlags = %08x}"),
                                    controller,
                                    _CUIControllerGetUsageScenarioString(controller->_usageScenario),
                                    (unsigned)controller->_usageFlags);
}

static const CFRuntimeClass _CUIControllerClass = {
    0,
    "CUICredential",
    NULL, // init
    NULL, // copy
    _CUIControllerDeallocate,
    NULL,
    NULL, // _CUICredentialHash,
    NULL, // _CUICredentialCopyFormattingDesc
    _CUIControllerCopyDescription
};

CUI_EXPORT CFTypeID
CUIControllerGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (_CUIControllerTypeID == _kCFRuntimeNotATypeID) {
            _CUIControllerTypeID = _CFRuntimeRegisterClass(&_CUIControllerClass);
        }
    });
    
    return _CUIControllerTypeID;
}

static Boolean
_CUIValidateUsageScenario(CUIUsageScenario usageScenario)
{
    return (usageScenario > kCUIUsageScenarioInvalid &&
            usageScenario <= kCUIUsageScenarioSSO);
}

CUI_EXPORT CUIControllerRef
CUIControllerCreate(CFAllocatorRef allocator,
                    CUIUsageScenario usageScenario,
                    CUIUsageFlags usageFlags)
{
    CUIControllerRef controller;

    assert(_CUIValidateUsageScenario(usageScenario));
   
    if (!_CUIValidateUsageScenario(usageScenario))
        return NULL;
 
    controller = (CUIControllerRef)_CFRuntimeCreateInstance(allocator,
                                                            CUIControllerGetTypeID(),
                                                            sizeof(struct __CUIController) - sizeof(CFRuntimeBase),
                                                            NULL);
    if (controller == NULL)
        return NULL;
    
    controller->_usageScenario = usageScenario;
    controller->_usageFlags = usageFlags;

    if (!_CUIProvidersCreate(allocator, controller)) {
        CFRelease(controller);
        return NULL;
    }
    
    controller->_attributes = CFDictionaryCreateMutable(allocator,
                                                        0,
                                                        &kCFTypeDictionaryKeyCallBacks,
                                                        &kCFTypeDictionaryValueCallBacks);
    if (controller->_attributes == NULL) {
        CFRelease(controller);
        return NULL;
    }
    
    return controller;
}

CUI_EXPORT CUIUsageScenario
CUIControllerGetUsageScenario(CUIControllerRef controller)
{
    return controller->_usageScenario;
}

CUI_EXPORT CUIUsageFlags
CUIControllerGetUsageFlags(CUIControllerRef controller)
{
    return controller->_usageFlags;
}

struct CUIEnumerateCredentialContext {
    CUIControllerRef controller;
    CFDictionaryRef attributes;
    CFDictionaryRef providerAttributes;
    CUIProvider *provider;
    void (^callback)(CUICredentialRef, Boolean, CFErrorRef);
    CFIndex index;
    CFIndex defaultCredentialIndex;
    Boolean didEnumerate;
};

static void
_CUIAssertionCheck(const CUIEnumerateCredentialContext *enumContext, CUICredentialRef cred)
{
    CFDictionaryRef attributes = CUICredentialGetAttributes(cred);

    assert(attributes);
    
    CFUUIDRef cred_persistenceID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrPersistenceFactoryID);
    CFUUIDRef prov_persistenceID = (CFUUIDRef)CFDictionaryGetValue(enumContext->providerAttributes, kCUIAttrPersistenceFactoryID);
    
    assert(cred_persistenceID == NULL || CFGetTypeID(cred_persistenceID) == CFUUIDGetTypeID());
    assert(!prov_persistenceID || (cred_persistenceID && CFEqual(cred_persistenceID, prov_persistenceID)));
    
    CFUUIDRef cred_factoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrProviderFactoryID);
    CFUUIDRef prov_factoryID = (CFUUIDRef)CFDictionaryGetValue(enumContext->providerAttributes, kCUIAttrProviderFactoryID);
    
    assert(cred_factoryID == NULL || CFGetTypeID(cred_factoryID) == CFUUIDGetTypeID());
    assert(!prov_factoryID || (cred_factoryID && CFEqual(cred_factoryID, prov_factoryID)));
}

static void
_CUIEnumerateMatchingCredentialsForProviderCallback(const void *value, void *_context)
{
    CUIEnumerateCredentialContext *enumContext = (CUIEnumerateCredentialContext *)_context;
    CUICredentialRef cred = (CUICredentialRef)value;

    assert(cred);
    _CUIAssertionCheck(enumContext, cred);
    
    enumContext->callback(cred, (enumContext->index == enumContext->defaultCredentialIndex), NULL);
    enumContext->didEnumerate = true;

    enumContext->index++;
}

static Boolean
_CUIControllerEnumerateMatchingCredentialsForProvider(CUIControllerRef controller,
                                                      CUIUsageFlags usageFlags,
                                                      CFDictionaryRef attributes,
                                                      CFDictionaryRef providerAttributes,
                                                      CUIProvider *provider,
                                                      void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    CFArrayRef matchingCreds;
    CFErrorRef error = NULL;
    Boolean providerExplicitlySupportsClass = false;
 
    CUIEnumerateCredentialContext enumContext = {
        .controller = controller,
        .attributes = attributes,
        .providerAttributes = providerAttributes,
        .provider = provider,
        .callback = cb,
        .didEnumerate = false,
        .index = 0,
        .defaultCredentialIndex = kCFNotFound
    };

    if (attributes) {
        CFArrayRef supportedClasses = (CFArrayRef)CFDictionaryGetValue(providerAttributes, kCUIAttrClass);
        CFTypeRef desiredClass = CFDictionaryGetValue(attributes, kCUIAttrClass);

        providerExplicitlySupportsClass =
            supportedClasses &&         /* provider advertised classes */
            desiredClass &&             /* caller desired class */
            CFArrayContainsValue(supportedClasses, CFRangeMake(0, CFArrayGetCount(supportedClasses)), desiredClass);
 
        if ((usageFlags & kCUIUsageFlagsInClassOnly) && !providerExplicitlySupportsClass)
            return false;
    }
    
    matchingCreds = provider->copyMatchingCredentials(attributes,
                                                      usageFlags,
                                                      &enumContext.defaultCredentialIndex,
                                                      &error);
    if (matchingCreds) {
        if (enumContext.defaultCredentialIndex == kCFNotFound && providerExplicitlySupportsClass)
            enumContext.defaultCredentialIndex = 0;

        CFArrayApplyFunction(matchingCreds,
                             CFRangeMake(0, CFArrayGetCount(matchingCreds)),
                             _CUIEnumerateMatchingCredentialsForProviderCallback,
                             (void *)&enumContext);
        
        CFRelease(matchingCreds);
    } else if (error) {
        cb(NULL, kCFNotFound, error);
        CFRelease(error);
    }
    
    return enumContext.didEnumerate;
}

static Boolean
_CUIControllerProviderMatch(CFDictionaryRef providerAttributes,
                            CFDictionaryRef assertedAttributes,
                            CFStringRef key,
                            CUIUsageFlags usageFlags,
                            CUIUsageFlags usageFlagIndicatingExclusion)
{
    CFTypeRef providerID = CFDictionaryGetValue(providerAttributes, key);

    if (providerID && usageFlagIndicatingExclusion &&
        (usageFlags & usageFlagIndicatingExclusion))
        return false;

    CFTypeRef assertedID = assertedAttributes ? CFDictionaryGetValue(assertedAttributes, key) : NULL;

    if (providerID && assertedID && !CFEqual(providerID, assertedID))
        return false;

    return true;
}

CUI_EXPORT Boolean
_CUIControllerEnumerateCredentialsWithFlags(CUIControllerRef controller,
                                            CUIUsageFlags extraUsageFlags,
                                            CFDictionaryRef attributes,
                                            void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    CFArrayRef items = NULL;
    CFErrorRef error = NULL;
    Boolean didEnumerate = false;
    CUIUsageFlags usageFlags = controller->_usageFlags | extraUsageFlags;
    
    /*
     * If kCUIUsageFlagsInCredOnly or kCUIUsageFlagsInClassOnly is specified, we must
     * have an input set of attributes.
     */
    if (controller->_attributes == NULL &&
        (usageFlags & (kCUIUsageFlagsInClassOnly | kCUIUsageFlagsInCredOnly)))
        return false;

    for (CFIndex index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        CFDictionaryRef providerAttributes = (CFDictionaryRef)CFArrayGetValueAtIndex(controller->_providerAttributes, index);
        CUIProvider *provider = (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);

        if (!_CUIControllerProviderMatch(providerAttributes, attributes, kCUIAttrPersistenceFactoryID, usageFlags, kCUIUsageFlagsExcludePersistedCreds) ||
            !_CUIControllerProviderMatch(providerAttributes, attributes, kCUIAttrIdentityFactoryID, usageFlags, kCUIUsageFlagsExcludeIdentityCreds))
            continue;

        if (!_CUIControllerProviderMatch(providerAttributes, attributes, kCUIAttrProviderFactoryID, usageFlags, 0))
            continue;

        didEnumerate |= _CUIControllerEnumerateMatchingCredentialsForProvider(controller,
                                                                              usageFlags,
                                                                              attributes,
                                                                              providerAttributes,
                                                                              provider,
                                                                              cb);
    }

    if (items)
        CFRelease(items);
    else if (error)
        CFRelease(error);
    
    return didEnumerate;
}

static Boolean
_CUIControllerCopyAttributesAdjustedForAuthError(CUIControllerRef controller,
                                                 CUIUsageFlags *extraUsageFlags,
                                                 CFDictionaryRef *adjustedAttributes)
{
    *adjustedAttributes = NULL;

    /*
     * This is to handle a special case which is not yet supported by Heimdal,
     * that is when a mechanism is in play to successfully authenticate but
     * needs more information from the user. In this case it will return
     * GSS_S_CONTINUE_NEEDED | GSS_S_PROMPTING_NEEDED.
     */
    if (CUIControllerGetUsageScenario(controller) == kCUIUsageScenarioNetwork &&    /* context is a GSS context handle */
        CUIControllerGetContext(controller) != GSS_C_NO_CONTEXT &&                  /* context in play */
        GSSIsPromptingNeeded(CUIControllerGetAuthError(controller)) &&              /* GSS_S_PROMPTING_NEEDED */
        !GSS_ERROR(CFErrorGetCode(CUIControllerGetAuthError(controller)))) {        /* GSS_S_CONTINUE_NEEDED or non-fatal */
        CFStringRef attrClass;

        attrClass = _CUICopyAttrClassForAuthError(controller->_authError);
        if (attrClass) {
            CFMutableDictionaryRef attributes;
            
            if (controller->_attributes)
                attributes = CFDictionaryCreateMutableCopy(CFGetAllocator(controller), 0, controller->_attributes);
            else
                attributes = CFDictionaryCreateMutable(CFGetAllocator(controller),
                                                       0,
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
            if (attributes == NULL) {
                CFRelease(attrClass);
                return false;
            }

            /* replace the class with that of the mechanism */            
            CFDictionarySetValue(attributes, kCUIAttrClass, attrClass);
            CFRelease(attrClass);

            *adjustedAttributes = attributes;
            /* ensure that only the mechanism class is enumerated */
            *extraUsageFlags |= kCUIUsageFlagsInClassOnly;
            return true;
        }
    }
    
    if (*adjustedAttributes == NULL && controller->_attributes)
        *adjustedAttributes = (CFDictionaryRef)CFRetain(controller->_attributes);
    
    return true;
}

CUI_EXPORT Boolean
CUIControllerEnumerateCredentials(CUIControllerRef controller, void (^cb)(CUICredentialRef, Boolean, CFErrorRef))
{
    Boolean ret;
    CFDictionaryRef attributes = NULL;
    CUIUsageFlags extraUsageFlags = 0;
 
    if (!_CUIControllerCopyAttributesAdjustedForAuthError(controller, &extraUsageFlags, &attributes))
        return false;
    
    ret = _CUIControllerEnumerateCredentialsWithFlags(controller, extraUsageFlags, attributes, cb);
    
    if (attributes)
        CFRelease(attributes);
    
    return ret;
}

static void
_CUICopyMutableAttributesKeys(const void *key, const void *value, void *context)
{
    // XXX probably should filter these
    CFDictionarySetValue((CFMutableDictionaryRef)context, key, value);
}

CUI_EXPORT void
CUIControllerSetAttributes(CUIControllerRef controller, CFDictionaryRef attributes)
{
    CFDictionaryApplyFunction(attributes, _CUICopyMutableAttributesKeys, (void *)controller->_attributes);
}

CUI_EXPORT CFDictionaryRef
CUIControllerGetAttributes(CUIControllerRef controller)
{
    return controller->_attributes;
}

CUI_EXPORT void
CUIControllerSetAuthError(CUIControllerRef controller, CFErrorRef authError)
{
    _CUISetter((CFTypeRef &)controller->_authError, authError);
}

CUI_EXPORT CFErrorRef
CUIControllerGetAuthError(CUIControllerRef controller)
{
    return controller->_authError;
}

CUI_EXPORT Boolean
CUIControllerSetCredUIContext(CUIControllerRef controller,
                              CUICredUIContextProperties whichProps,
                              const CUICredUIContext *uic)
{
    CUICredUIContext *cuic = &controller->_uiContext;
    
    if (uic->version != 0)
        return false;
    
    if (whichProps & kCUICredUIContextPropertyParentWindow)
        _CUISetter(cuic->parentWindow, uic->parentWindow);
    if (whichProps & kCUICredUIContextPropertyMessageText)
        _CUISetter((CFTypeRef &)cuic->messageText, (CFTypeRef)uic->messageText);
    if (whichProps & kCUICredUIContextPropertyTitleText)
        _CUISetter((CFTypeRef &)cuic->titleText, (CFTypeRef)uic->titleText);
    
    return true;
}

CUI_EXPORT const CUICredUIContext *
CUIControllerGetCredUIContext(CUIControllerRef controller)
{
    return &controller->_uiContext;
}

CUI_EXPORT void
CUIControllerSetContext(CUIControllerRef controller, const void *context)
{
    controller->_context = context;
}

CUI_EXPORT const void *
CUIControllerGetContext(CUIControllerRef controller)
{
    return controller->_context;
}

CUI_EXPORT void
CUIControllerSetTargetName(CUIControllerRef controller, CFTypeRef targetName)
{
    _CUISetter((CFTypeRef &)controller->_targetName, targetName);
}

CUI_EXPORT CFTypeRef
CUIControllerGetTargetName(CUIControllerRef controller)
{
    return controller->_targetName;
}

CUI_EXPORT CFStringRef
CUICopyTargetDisplayName(CFTypeRef targetName)
{
    CFStringRef displayName = NULL;
    CFTypeID targetNameTypeID;
    
    if (targetName == NULL)
        return NULL;
    
    targetNameTypeID = CFGetTypeID(targetName);
    if (targetNameTypeID == CFURLGetTypeID()) {
        displayName = (CFStringRef)CFRetain(CFURLGetString((CFURLRef)targetName));
    } else if (targetNameTypeID == CFStringGetTypeID()) {
        displayName = (CFStringRef)CFRetain(targetName);
    } else {
        /* here's hoping it's a GSS name, because otherwise we will crash XXX */
        displayName = GSSNameCreateDisplayString((gss_name_t)targetName);
    }
    
    return displayName;
}

static CFStringRef
_GSSServiceNameCreateDisplayString(gss_name_t targetName)
{
    OM_uint32 major, minor;
    gss_buffer_desc buffer = GSS_C_EMPTY_BUFFER;
    gss_OID oid = GSS_C_NO_OID;
    CFStringRef displayString, hostName = NULL;
    
    major = gss_display_name(&minor, targetName, &buffer, &oid);
    if (GSS_ERROR(major) || !gss_oid_equal(oid, GSS_C_NT_HOSTBASED_SERVICE))
        return NULL;
   
    displayString = CUICreateStringWithGSSBuffer(buffer);
    (void) gss_release_buffer(&minor, &buffer);

    if (displayString) {
        CFRange range = CFStringFind(displayString, CFSTR("@"), 0);
        if (range.location != kCFNotFound && range.length > 0) {
            range.location++;
            range.length = CFStringGetLength(displayString) - range.location;
            
            hostName = CFStringCreateWithSubstring(kCFAllocatorDefault, displayString, range);
        }
        
        CFRelease(displayString);
    }
    
    return hostName;
}

CUI_EXPORT CFStringRef
CUICopyTargetHostName(CFTypeRef targetName)
{
    CFStringRef hostName = NULL;
    
    if (targetName == NULL)
        return NULL;
    
    if (CFGetTypeID(targetName) == CFURLGetTypeID()) {
        hostName = CFURLCopyHostName((CFURLRef)targetName);
    } else if (CFGetTypeID(targetName) != CFStringGetTypeID()) {
        /* here's hoping it's a GSS name, because otherwise we will crash XXX */
        hostName = _GSSServiceNameCreateDisplayString((gss_name_t)targetName);
    }
    
    return hostName;
}

void
_CUIControllerShowProviders(CUIControllerRef controller)
{
    CFIndex index;
    
    assert(CFArrayGetCount(controller->_providerAttributes) == CFArrayGetCount(controller->_providers));
    
    if (controller->_providerAttributes) {
        for (index = 0; index < CFArrayGetCount(controller->_providerAttributes); index++) {
            CFShow(CFArrayGetValueAtIndex(controller->_providerAttributes, index));
        }
    }
}

static void
_CUICFSetExtendApplyFunction(const void *value, void *context)
{
    CFMutableSetRef theSet = (CFMutableSetRef)context;
    CFSetSetValue(theSet, value);
}

static void
_CUICFSetExtend(CFMutableSetRef theSet, CFSetRef anotherSet)
{
    CFSetApplyFunction(anotherSet, _CUICFSetExtendApplyFunction, theSet);
}

static CFStringRef const _CUIDefaultWhitelistedAttributeKeys[] = {
    kCUIAttrClass,
    kCUIAttrSupportGSSCredential,
    kCUIAttrNameType,
    kCUIAttrName,
    kCUIAttrNameDisplay,
    kCUIAttrUUID,
    kCUIAttrTransientExpire,
    kCUIAttrTransientDefaultInClass,
    kCUIAttrCredentialSecIdentity,
    kCUIAttrCredentialSecCertificate,
    kCUIAttrCredentialExists,
    kCUIAttrGSSItem,
    kCUIAttrSecKeychainItem,
    kCUIAttrCSIdentity,
    kCUIAttrGSSCredential,
    kCUIAttrImageData,
    kCUIAttrImageDataType,
    kCUIAttrCredentialError,
    kCUIAttrAuthenticatedForLoginScenario
};

CUI_EXPORT CFSetRef
_CUIControllerCopyWhitelistedAttributeKeys(CUIControllerRef controller)
{
    static CFSetRef defaultWhitelist;
    static dispatch_once_t onceToken;
    CFMutableSetRef whitelist;

    dispatch_once(&onceToken, ^{
        defaultWhitelist = CFSetCreate(kCFAllocatorDefault,
                                       (const void **)_CUIDefaultWhitelistedAttributeKeys,
                                       sizeof(_CUIDefaultWhitelistedAttributeKeys) / sizeof(_CUIDefaultWhitelistedAttributeKeys[0]),
                                       &kCFTypeSetCallBacks);
    });

    whitelist = CFSetCreateMutable(kCFAllocatorDefault, 0, &kCFTypeSetCallBacks);
    _CUICFSetExtend(whitelist, defaultWhitelist);

    for (CFIndex index = 0; index < CFArrayGetCount(controller->_providers); index++) {
        CUIProvider *provider = (CUIProvider *)CFArrayGetValueAtIndex(controller->_providers, index);
        CFSetRef providerWhitelist;

        providerWhitelist = provider->getWhitelistedAttributeKeys();
        if (providerWhitelist)
            _CUICFSetExtend(whitelist, providerWhitelist);
    }

    return whitelist;
}

