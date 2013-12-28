//
//  CUIField.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIField__
#define __CredUI__CUIField__

#ifdef __cplusplus
extern "C" {
#endif
    
#include <CoreFoundation/CoreFoundation.h>
    
typedef struct __CUIField *CUIFieldRef;

typedef CF_ENUM(CFIndex, CUIFieldClass) {
    kCUIFieldClassInvalid = 0,
    kCUIFieldClassLargeText,
    kCUIFieldClassSmallText,
    kCUIFieldClassCommandLink,
    kCUIFieldClassEditText,
    kCUIFieldClassPasswordText,
    kCUIFieldClassTileImage,
    kCUIFieldClassCheckbox,
    kCUIFieldClassCombobox,
    kCUIFieldClassSubmitButton
};

extern CFTypeID
CUIFieldGetTypeID(void);
    
extern CUIFieldRef
CUIFieldCreate(
    CFAllocatorRef allocator,
    CUIFieldClass fieldClass,
    CFStringRef title,
    void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value));

extern CUIFieldRef
CUIFieldCreateCopy(
    CFAllocatorRef allocator,
    CUIFieldRef field);
    
extern CUIFieldClass
CUIFieldGetClass(CUIFieldRef field);

extern CFTypeRef
CUIFieldGetValue(CUIFieldRef field);
    
extern CFTypeRef
CUIFieldGetTitle(CUIFieldRef field);

extern void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value);
    
#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIField__) */