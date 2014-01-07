//
//  CUICredential.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

struct __CUICredential {
    CFRuntimeBase _base;
    CUICredentialContext *_context;
};

static void
_CUICredentialContextRelease(CFAllocatorRef allocator, const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    credContext->Release();
}

static CFStringRef
_CUICredentialContextCopyDescription(const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    return credContext->copyDescription();
}

static Boolean
_CUICredentialContextEqual(const void *value1, const void *value2)
{
    CUICredentialContext *credContext1 = (CUICredentialContext *)value1;
    CUICredentialContext *credContext2 = (CUICredentialContext *)value2;
    
    return (credContext1 == credContext2);
}

static CFTypeID __CUICredentialTypeID = _kCFRuntimeNotATypeID;

static void _CUICredentialDeallocate(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;

    if (cred->_context)
        _CUICredentialContextRelease(CFGetAllocator(cf), cred->_context);
}

static Boolean _CUICredentialEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    CUICredentialRef cred1 = (CUICredentialRef)cf1;
    CUICredentialRef cred2 = (CUICredentialRef)cf2;
    Boolean equals;
    
    equals = (cred1 == cred2);
    if (!equals)
        equals = _CUICredentialContextEqual(cred1->_context, cred2->_context);
    
    return equals;
}

static CFStringRef _CUICredentialCopyDescription(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;
    CFStringRef desc = NULL;
  
    if (cred->_context)
        desc = _CUICredentialContextCopyDescription(cred->_context);
    
    if (desc == NULL) {
        desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                        CFSTR("<CUICredential %p>{context = %p}"), cred, cred->_context);
    }
    
    return desc;
}

static const CFRuntimeClass _CUICredentialClass = {
    0,
    "CUICredential",
    NULL, // init
    NULL, // copy
    _CUICredentialDeallocate,
    _CUICredentialEqual,
    NULL, // _CUICredentialHash,
    NULL, // _CUICredentialCopyFormattingDesc
    _CUICredentialCopyDescription
};

CUI_EXPORT CFTypeID
CUICredentialGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (__CUICredentialTypeID == _kCFRuntimeNotATypeID) {
            __CUICredentialTypeID = _CFRuntimeRegisterClass(&_CUICredentialClass);
            _CFRuntimeBridgeClasses(__CUICredentialTypeID, "CUICFCredential");
        }
    });
    
    return __CUICredentialTypeID;
}

CUI_EXPORT CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknown *iunk)
{
    CUICredentialRef cred;
    
    cred = (CUICredentialRef)_CFRuntimeCreateInstance(allocator, CUICredentialGetTypeID(),
                                                      sizeof(struct __CUICredential) - sizeof(CFRuntimeBase), NULL);
    if (cred == NULL)
        return NULL;
    
    if (iunk) {
        CUICredentialContext *credContext;
        
        iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUICredentialInterfaceID), (void **)&credContext);
        if (credContext == NULL) {
            CFRelease(cred);
            return NULL;
        }
        
        cred->_context = credContext;
    }
    
    return cred;
}

CUI_EXPORT CFArrayRef
CUICredentialGetFields(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, CFArrayRef, cred, "fields");
    
    if (cred->_context)
        return cred->_context->getFields();
    
    return NULL;
}

CUI_EXPORT CFDictionaryRef
CUICredentialGetAttributes(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, CFDictionaryRef, cred, "attributes");
    
    if (cred->_context)
        return cred->_context->getAttributes();
    
    return NULL;
}

CUI_EXPORT void
CUICredentialDidBecomeSelected(CUICredentialRef cred, Boolean *pbAutoLogin)
{
    if (CF_IS_OBJC(__CUICredentialTypeID, cred)) {
        BOOL bAutoLogin;

        CF_OBJC_VOIDCALLV(cred, "didBecomeSelected:", &bAutoLogin);
        *pbAutoLogin = (Boolean)bAutoLogin;
    } else if (cred->_context) {
        cred->_context->didBecomeSelected(pbAutoLogin);
    }
}

static Boolean
_CUICredentialHasMandatoryKeys(CUICredentialRef cred)
{
    CFDictionaryRef attrs = CUICredentialGetAttributes(cred);
    
    // probably should actually validate some values too
    return CFDictionaryGetValue(attrs, kCUIAttrCredentialStatus) &&
           CFDictionaryGetValue(attrs, kCUIAttrName) &&
           CFDictionaryGetValue(attrs, kCUIAttrNameType);
}

static Boolean
_CUICredentialIsReturnable(CUICredentialRef cred)
{
    CFDictionaryRef attrs = CUICredentialGetAttributes(cred);
    CFTypeRef status = CFDictionaryGetValue(attrs, kCUIAttrCredentialStatus);
    
    if (status) {
        if (CFEqual(status, kCUICredentialReturnCredentialFinished) ||
            CFEqual(status, kCUICredentialReturnNoCredentialFinished))
            return true;
    }
    
    return false;
}

CUI_EXPORT Boolean
CUICredentialCanSubmit(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, Boolean, cred, "canSubmit");

    return _CUICredentialHasMandatoryKeys(cred) &&
           _CUICredentialIsReturnable(cred);
}

CUI_EXPORT void
CUICredentialWillSubmit(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, void, cred, "willSubmit");

    CUIFieldRef selectedCredSubmitButton;
   
    selectedCredSubmitButton = CUICredentialFindFirstFieldWithClass(cred, kCUIFieldClassSubmitButton);
    if (selectedCredSubmitButton)
        CUIFieldSetValue(selectedCredSubmitButton, kCFBooleanTrue);
}

CUI_EXPORT void
CUICredentialDidSubmit(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, void, cred, "didSubmit");

    if (cred->_context)
        cred->_context->didSubmit();
}

CUI_EXPORT void
CUICredentialDidBecomeDeselected(CUICredentialRef cred)
{
    CF_OBJC_FUNCDISPATCHV(__CUICredentialTypeID, void, cred, "didBecomeDeselected");

    if (cred->_context)
        cred->_context->didBecomeDeselected();
}

struct CUICredentialFilterFieldsContext {
    Boolean (^predicate)(CUIFieldRef);
    CFMutableArrayRef array;
};

static void
_CUICredentialFilterFieldsWithPredicate(const void *value, void *_context)
{
    CUIFieldRef field = (CUIFieldRef)value;
    CUICredentialFilterFieldsContext *context = (CUICredentialFilterFieldsContext *)_context;
    
    if (context->predicate(field))
        CFArrayAppendValue(context->array, field);
}

CUI_EXPORT CFArrayRef
CUICredentialCopyFieldsWithPredicate(CUICredentialRef cred,
                                     Boolean (^predicate)(CUIFieldRef field))
{
    CFArrayRef fields = CUICredentialGetFields(cred);
    CUICredentialFilterFieldsContext context;
    
    if (fields == NULL)
        return NULL;

    context.predicate = predicate;
    context.array = CFArrayCreateMutable(CFGetAllocator(fields), 0, &kCFTypeArrayCallBacks);

    CFArrayApplyFunction(fields,
                         CFRangeMake(0, CFArrayGetCount(fields)),
                         _CUICredentialFilterFieldsWithPredicate,
                         (void *)&context);
    
    return context.array;
}

CUI_EXPORT void
CUICredentialFieldsApplyBlock(CUICredentialRef cred, void (^cb)(CUIFieldRef, Boolean *stop))
{
    CFArrayRef fields = CUICredentialGetFields(cred);
    CFIndex index;
    Boolean stop = false;
    
    if (fields) {
        for (index = 0; index < CFArrayGetCount(fields); index++) {
            CUIFieldRef field = (CUIFieldRef)CFArrayGetValueAtIndex(fields, index);
            
            cb(field, &stop);
            if (stop)
                break;
        }
    }
}

CUI_EXPORT CUIFieldRef
CUICredentialFindFirstFieldWithClass(CUICredentialRef cred, CUIFieldClass fieldClass)
{
    __block CUIFieldRef theField = NULL;

    CUICredentialFieldsApplyBlock(cred, ^(CUIFieldRef field, Boolean *stop) {
        if (CUIFieldGetClass(field) == fieldClass) {
            theField = (CUIFieldRef)CFRetain(field);
            *stop = true;
        }
    });
    
    return theField;
}

CUI_EXPORT Boolean
CUICredentialSavePersisted(CUICredentialRef cred, CFErrorRef *error)
{
    Boolean bRet = false;
    
    if (error)
        *error = NULL;

    if (CF_IS_OBJC(__CUICredentialTypeID, cred)) {
        CF_OBJC_VOIDCALLV(cred, "savePersisted:", error);
        if (error && *error)
            CFRetain(*error);
    } else if (cred->_context) {
        bRet = cred->_context->savePersisted(error);
    }

    return bRet;
}

CUI_EXPORT Boolean
CUICredentialDeletePersisted(CUICredentialRef cred, CFErrorRef *error)
{
    Boolean bRet = false;
    
    if (error)
        *error = NULL;
    
    if (CF_IS_OBJC(__CUICredentialTypeID, cred)) {
        CF_OBJC_VOIDCALLV(cred, "deletePersisted:", error);
        if (error && *error)
            CFRetain(*error);
    } else if (cred->_context) {
        bRet = cred->_context->deletePersisted(error);
    }
    
    return bRet;
}
