//
//  CUIField.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

static CFTypeID __CUIFieldTypeID = _kCFRuntimeNotATypeID;
static CFStringRef __CUIFieldValueProperty = CFSTR("value");
static CFStringRef __CUIFieldOptionsProperty = CFSTR("options");

CF_INLINE Boolean __CFEqualNullSafe(CFTypeRef cf1, CFTypeRef cf2)
{
    if (cf1 && cf2 == NULL)
        return false;
    if (cf2 && cf1 == NULL)
        return false;
    if (cf1)
        return CFEqual(cf1, cf2);
    
    return true;
}

static CFTypeRef _CUIFieldCopy(CFAllocatorRef allocator, CFTypeRef cf)
{
    return (CFTypeRef)CUIFieldCreateCopy(allocator, (CUIFieldRef)cf);
}

static void _CUIFieldDeallocate(CFTypeRef cf)
{
    CUIFieldRef field = (CUIFieldRef)cf;
    
    if (field->_title)
        CFRelease(field->_title);
    if (field->_defaultValue)
        CFRelease(field->_defaultValue);
    if (field->_delegate)
        _Block_release(field->_delegate);
}

static Boolean _CUIFieldEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    Boolean equal;
    CUIFieldRef f1 = (CUIFieldRef)cf1, f2 = (CUIFieldRef)cf2;
    
    equal = (cf1 == cf2);
    
    if (!equal) {
        equal = (f1->_class == f2->_class) &&
        __CFEqualNullSafe(f1->_title, f2->_title) &&
        __CFEqualNullSafe(f1->_defaultValue, f2->_defaultValue);
    }
    
    return equal;
    
}

static CFStringRef _CUIFieldCopyDescription(CFTypeRef cf)
{
    CUIFieldRef field = (CUIFieldRef)cf;
    CFStringRef desc;
    
    desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                    CFSTR("<CUIField %p>{class = %d, title = \"%@\", options = %08x}"),
                                    field,
                                    (int)field->_class, field->_title, (unsigned int)field->_options);
    
    return desc;
}

static const CFRuntimeClass _CUIFieldClass = {
    0,
    "CUIField",
    NULL, // init
    _CUIFieldCopy, // copy
    _CUIFieldDeallocate,
    _CUIFieldEqual,
    NULL, // _CUIFieldHash,
    NULL, // _CUICopyFormattingDesc
    _CUIFieldCopyDescription
};

CUI_EXPORT CFTypeID
CUIFieldGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (__CUIFieldTypeID == _kCFRuntimeNotATypeID) {
            __CUIFieldTypeID = _CFRuntimeRegisterClass(&_CUIFieldClass);
            _CFRuntimeBridgeClasses(__CUIFieldTypeID, "CUICFField");
        }
    });
    
    return __CUIFieldTypeID;
}

CUI_EXPORT CUIFieldRef
CUIFieldCreate(CFAllocatorRef allocator,
               CUIFieldClass fieldClass,
               CFStringRef title,
               CFTypeRef defaultValue,
               void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value))
{
    CUIFieldRef f;
    
    f = (CUIFieldRef)_CFRuntimeCreateInstance(allocator, CUIFieldGetTypeID(),
                                              sizeof(struct __CUIField) - sizeof(CFRuntimeBase), NULL);
    if (f == NULL)
        return NULL;
    
    f->_class = fieldClass;
    f->_title = title ? (CFStringRef)CFRetain(title) : NULL;
    f->_defaultValue = defaultValue ? CFRetain(defaultValue) : NULL;
    f->_delegate = (void (^)(CUIFieldRef, CFTypeRef))_Block_copy(fieldDidChange);
    
    return f;
}

CUI_EXPORT CUIFieldRef
CUIFieldCreateCopy(CFAllocatorRef allocator,
                   CUIFieldRef field)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, CUIFieldRef, field, "copy");
    
    CUIFieldRef f = CUIFieldCreate(allocator, field->_class, field->_title, field->_defaultValue, field->_delegate);
    if (f == NULL)
        return NULL;
    
    return f;
}

CUI_EXPORT CUIFieldClass
CUIFieldGetClass(CUIFieldRef field)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, CUIFieldClass, field, "fieldClass");

    return field->_class;
}

CUI_EXPORT CFStringRef
CUIFieldGetTitle(CUIFieldRef field)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, CFStringRef, field, "title");

    return field->_title;
}

CUI_EXPORT CFTypeRef
CUIFieldGetDefaultValue(CUIFieldRef field)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, CFTypeRef, field, "defaultValue");

    return field->_defaultValue;
}

CUI_EXPORT void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, void, field, "setValue:", value);

    CF_OBJC_KVO_WILLCHANGE(field, __CUIFieldValueProperty);
    if (field->_delegate)
        field->_delegate(field, value);
    CF_OBJC_KVO_DIDCHANGE(field, __CUIFieldValueProperty);
}

CUI_EXPORT void
CUIFieldSetOptions(CUIFieldRef field, CUIFieldOptions value)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, void, field, "setOptions:", value);
    
    CF_OBJC_KVO_WILLCHANGE(field, __CUIFieldOptionsProperty);
    field->_options = value;
    CF_OBJC_KVO_DIDCHANGE(field, __CUIFieldOptionsProperty);
}

CUI_EXPORT CUIFieldOptions
CUIFieldGetOptions(CUIFieldRef field)
{
    CF_OBJC_FUNCDISPATCHV(__CUIFieldTypeID, BOOL, field, "options");
    
    return field->_options;
}
