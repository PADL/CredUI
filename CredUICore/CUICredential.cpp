
//
//  CUICredential.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static const void *
_CUICredentialContextRetain(CFAllocatorRef allocator, const void *value)
{
    CUICredentialContext *credContext = (CUICredentialContext *)value;
    credContext->AddRef();
    return credContext;
}

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

CFArrayCallBacks kCUICredentialContextArrayCallBacks = {
    .version = 0,
    .retain = _CUICredentialContextRetain,
    .release = _CUICredentialContextRelease,
    .copyDescription = _CUICredentialContextCopyDescription,
    .equal = _CUICredentialContextEqual
};

struct __CUICredential {
    CFRuntimeBase _base;
    CUICredentialContext *_context;
    GSSItemRef _gssItem;
};

static CFTypeID _CUICredentialTypeID = _kCFRuntimeNotATypeID;

static void _CUICredentialDeallocate(CFTypeRef cf)
{
    CUICredentialRef cred = (CUICredentialRef)cf;

    if (cred->_context)
        _CUICredentialContextRelease(CFGetAllocator(cf), cred->_context);
    if (cred->_gssItem)
        CFRelease(cred->_gssItem);
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

CFTypeID
CUICredentialGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (_CUICredentialTypeID == _kCFRuntimeNotATypeID) {
            _CUICredentialTypeID = _CFRuntimeRegisterClass(&_CUICredentialClass);
        }
    });
    
    return _CUICredentialTypeID;
}

CUICredentialRef
CUICredentialCreate(CFAllocatorRef allocator, IUnknown *iunk)
{
    CUICredentialRef cred;
    CUICredentialContext *credContext;
    
    cred = (CUICredentialRef)_CFRuntimeCreateInstance(allocator, CUICredentialGetTypeID(), sizeof(struct __CUICredential) - sizeof(CFRuntimeBase), NULL);
    if (cred == NULL)
        return NULL;
    
    iunk->QueryInterface(CFUUIDGetUUIDBytes(kCUICredentialInterfaceID), (void **)&credContext);
    if (credContext == NULL) {
        CFRelease(cred);
        return NULL;
    }
    
    cred->_context = credContext;

    return cred;
}

CFArrayRef
CUICredentialGetFields(CUICredentialRef cred)
{
    if (cred->_context)
        return cred->_context->getFields();
    
    return NULL;
}

CFDictionaryRef
CUICredentialGetAttributes(CUICredentialRef cred)
{
    if (cred->_context)
        return cred->_context->getAttributes();
    
    return NULL;
}

void
CUICredentialDidBecomeSelected(CUICredentialRef cred)
{
    if (cred->_context)
        cred->_context->didBecomeSelected();
}

struct __CUICredentialFilterFieldsContext {
    Boolean (^predicate)(CUIFieldRef);
    CFMutableArrayRef array;
};

static void
__CUICredentialFilterFieldsWithPredicate(const void *value, void *_context)
{
    CUIFieldRef field = (CUIFieldRef)value;
    __CUICredentialFilterFieldsContext *context = (__CUICredentialFilterFieldsContext *)_context;
    
    if (context->predicate(field))
        CFArrayAppendValue(context->array, field);
}

CFArrayRef
CUICredentialCopyFieldsWithPredicate(CUICredentialRef cred,
                                     Boolean (^predicate)(CUIFieldRef field))
{
    CFArrayRef fields = CUICredentialGetFields(cred);
    __CUICredentialFilterFieldsContext context;
    
    if (fields == NULL)
        return NULL;

    context.predicate = predicate;
    context.array = CFArrayCreateMutable(CFGetAllocator(fields), 0, &kCFTypeArrayCallBacks);

    CFArrayApplyFunction(fields,
                         CFRangeMake(0, CFArrayGetCount(fields)),
                         __CUICredentialFilterFieldsWithPredicate,
                         (void *)&context);
    
    return context.array;
}

void
CUICredentialFieldsApplyBlock(CUICredentialRef cred, void (^cb)(CUIFieldRef, Boolean *stop), Boolean *stop)
{
    CFArrayRef fields = CUICredentialGetFields(cred);
    CFIndex index;
    
    if (fields) {
        for (index = 0; index < CFArrayGetCount(fields); index++) {
            CUIFieldRef field = (CUIFieldRef)CFArrayGetValueAtIndex(fields, index);
            
            cb(field, stop);
            if (*stop)
                break;
        }
    }
}

CUIFieldRef
CUICredentialFindFirstFieldWithClass(CUICredentialRef cred, CUIFieldClass fieldClass)
{
    Boolean stop = false;
    __block CUIFieldRef theField = NULL;

    CUICredentialFieldsApplyBlock(cred, ^(CUIFieldRef field, Boolean *stop) {
        if (CUIFieldGetClass(field) == fieldClass) {
            theField = (CUIFieldRef)CFRetain(field);
            *stop = true;
        }
    }, &stop);
    
    return theField;
}

void
__CUICredentialSetItem(CUICredentialRef cred, GSSItemRef item)
{
    __CUISetter((CFTypeRef &)cred->_gssItem, item);
}

GSSItemRef
CUICredentialCreateGSSItem(CUICredentialRef cred, Boolean addIfNotExisting, CFErrorRef *pError)
{
    if (cred->_gssItem == NULL && addIfNotExisting) {
        CFDictionaryRef attributes = CUICredentialGetAttributes(cred);
        if (attributes)
            cred->_gssItem = GSSItemAdd(attributes, pError);
    }

    return cred->_gssItem;
}