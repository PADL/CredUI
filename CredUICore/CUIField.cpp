//
//  CUIField.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore_Private.h"

struct __CUIField {
    CFRuntimeBase _base;
    CUIFieldClass _class;
    CFStringRef _title;
    CFTypeRef _defaultValue;
    void (^_delegate)(CUIFieldRef field, CFTypeRef value);

};

static CFTypeID _CUIFieldTypeID = _kCFRuntimeNotATypeID;

// from CFXMLNode.c
CF_INLINE Boolean _nullSafeCFEqual(CFTypeRef cf1, CFTypeRef cf2)
{
    if (cf1 && !cf2) return false;
    if (cf2 && !cf1) return false;
    if (cf1) return CFEqual(cf1, cf2);
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
        _nullSafeCFEqual(f1->_title, f2->_title) &&
        _nullSafeCFEqual(f1->_defaultValue, f2->_defaultValue);
    }
    
    return equal;
    
}

static CFStringRef _CUIFieldCopyDescription(CFTypeRef cf)
{
    CUIFieldRef field = (CUIFieldRef)cf;
    CFStringRef desc;
    
    desc = CFStringCreateWithFormat(CFGetAllocator(cf), NULL,
                                    CFSTR("<CUIField %p>{class = %d, title = \"%@\"}"),
                                    field,
                                    (int)field->_class, field->_title);
    
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

CFTypeID
CUIFieldGetTypeID(void)
{
    static dispatch_once_t onceToken;
    
    dispatch_once(&onceToken, ^{
        if (_CUIFieldTypeID == _kCFRuntimeNotATypeID) {
            _CUIFieldTypeID = _CFRuntimeRegisterClass(&_CUIFieldClass);
        }
    });
    
    return _CUIFieldTypeID;
}

CUIFieldRef
CUIFieldCreate(
               CFAllocatorRef allocator,
               CUIFieldClass fieldClass,
               CFStringRef title,
               CFTypeRef defaultValue,
               void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value))
{
    CUIFieldRef f;
    
    f = (CUIFieldRef)_CFRuntimeCreateInstance(allocator, CUIFieldGetTypeID(), sizeof(struct __CUIField) - sizeof(CFRuntimeBase), NULL);
    if (f == NULL)
        return NULL;
    
    f->_class = fieldClass;
    f->_title = (CFStringRef)CFRetain(title);
    f->_defaultValue = defaultValue ? CFRetain(defaultValue) : NULL;
    f->_delegate = (void (^)(CUIFieldRef, CFTypeRef))_Block_copy(fieldDidChange);
    
    return f;
}

CUIFieldRef
CUIFieldCreateCopy(
                   CFAllocatorRef allocator,
                   CUIFieldRef field)
{
    CUIFieldRef f;
    
    f = CUIFieldCreate(allocator, field->_class, field->_title, field->_defaultValue, field->_delegate);
    if (f == NULL)
        return NULL;
    
    return f;
}

CUIFieldClass
CUIFieldGetClass(CUIFieldRef field)
{
    return field->_class;
}

CFStringRef
CUIFieldGetTitle(CUIFieldRef field)
{
    return field->_title;
}

CFTypeRef
CUIFieldGetDefaultValue(CUIFieldRef field)
{
    return field->_defaultValue;
}

void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value)
{
    if (field->_delegate)
        field->_delegate(field, value);
}
