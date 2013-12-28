//
//  CUIField.cpp
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#include "CredUICore.h"

struct __CUIField {
    CFRuntimeBase _base;
    CUIFieldClass _class;
    CFStringRef _title;
    CFTypeRef _value;
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

static void _CUIFieldDeallocate(CFTypeRef cf)
{
    CUIFieldRef field = (CUIFieldRef)cf;
    
    if (field->_title)
        CFRelease(field->_title);
    if (field->_value)
        CFRelease(field->_value);
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
            _nullSafeCFEqual(f1->_value, f2->_value);
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
    NULL, // copy
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
               void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value))
{
    CUIFieldRef f;
    
    f = (CUIFieldRef)_CFRuntimeCreateInstance(allocator, CUIFieldGetTypeID(), sizeof(struct __CUIField) - sizeof(CFRuntimeBase), NULL);
    if (f == NULL)
        return NULL;
    
    return f;
}

CUIFieldRef
CUIFieldCreateCopy(
                   CFAllocatorRef allocator,
                   CUIFieldRef field)
{
    CUIFieldRef f;
    
    f = CUIFieldCreate(allocator, field->_class, field->_title, field->_delegate);
    if (f == NULL)
        return NULL;
    
    CUIFieldSetValue(f, field->_value);
    
    return f;
}

CUIFieldClass
CUIFieldGetClass(CUIFieldRef field)
{
    return field->_class;
}

CFTypeRef
CUIFieldGetValue(CUIFieldRef field)
{
    return field->_value;
}

CFTypeRef
CUIFieldGetTitle(CUIFieldRef field)
{
    return field->_title;
}

void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value)
{
    if (field->_value != value) {
        CFRelease(field->_value);
        field->_value = CFRetain(value);
        
        field->_delegate(field, field->_value);
    }
}
