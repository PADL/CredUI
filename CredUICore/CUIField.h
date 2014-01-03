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
    
typedef struct __CUIField *CUIFieldRef;

typedef CF_ENUM(CFIndex, CUIFieldClass) {
    kCUIFieldClassInvalid = 0,
    kCUIFieldClassLargeText,
    kCUIFieldClassSmallText,
    kCUIFieldClassCommandLink,
    kCUIFieldClassEditText,
    kCUIFieldClassPasswordText,
    kCUIFieldClassTileImage,
    kCUIFieldClassCheckBox,
    kCUIFieldClassComboBox,
    kCUIFieldClassSubmitButton
};

extern CFTypeID
CUIFieldGetTypeID(void);
    
extern CUIFieldRef
CUIFieldCreate(
    CFAllocatorRef allocator,
    CUIFieldClass fieldClass,
    CFStringRef title,
    CFTypeRef defaultValue,
    void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value));

extern CUIFieldRef
CUIFieldCreateCopy(
    CFAllocatorRef allocator,
    CUIFieldRef field);
    
extern CUIFieldClass
CUIFieldGetClass(CUIFieldRef field);
    
extern CFStringRef
CUIFieldGetTitle(CUIFieldRef field);
    
extern CFTypeRef
CUIFieldGetDefaultValue(CUIFieldRef field);
    
extern void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value);

const void
CUIFieldSetHidden(CUIFieldRef field, Boolean value);

Boolean
CUIFieldGetIsHidden(CUIFieldRef field);

#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIField__) */
