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
    kCUIFieldClassSubmitButton,
    kCUIFieldClassCustom = 0x10000000
};

typedef CF_OPTIONS(CFIndex, CUIFieldOptions) {
    kCUIFieldOptionsNone                    = 0x00000000,
#if 0
    kCUIFieldOptionsEnablePasswordReveal    = 0x00000001,
    kCUIFieldOptionsIsEmailAddress          = 0x00000002,
#endif
    kCUIFieldOptionsIsHidden                = 0x10000000
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

extern void
CUIFieldSetOptions(CUIFieldRef field, CUIFieldOptions value);

CUIFieldOptions
CUIFieldGetOptions(CUIFieldRef field);

#ifdef __cplusplus
}
#endif

#endif /* defined(__CredUI__CUIField__) */
