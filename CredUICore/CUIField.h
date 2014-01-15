//
//  CUIField.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUICore_CUIField_h
#define CredUICore_CUIField_h

#include <CredUICore/CUIBase.h>

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

/*
 * Returns the type ID for a CUIField.
 */
CUI_EXPORT CFTypeID
CUIFieldGetTypeID(void);
  
/*
 * Creates a new field of a particular class. title and defaultValue may be NULL depending
 * on the type of field. The fieldDidChange block is called whenever the application updates
 * the field with CUIFieldSetValue. This function is only called by credential providers.
 */  
CUI_EXPORT CUIFieldRef
CUIFieldCreate(
    CFAllocatorRef allocator,
    CUIFieldClass fieldClass,
    CFStringRef title,
    CFTypeRef defaultValue,
    void (^fieldDidChange)(CUIFieldRef field, CFTypeRef value));

/*
 * Creates a copy of a field.
 */
CUI_EXPORT CUIFieldRef
CUIFieldCreateCopy(
    CFAllocatorRef allocator,
    CUIFieldRef field);

/*
 * Returns the type of field.
 */  
CUI_EXPORT CUIFieldClass
CUIFieldGetClass(CUIFieldRef field);
  
/*
 * Returns the title of the field.
 */  
CUI_EXPORT CFStringRef
CUIFieldGetTitle(CUIFieldRef field);
   
/*
 * Returns the default value of the field.
 */ 
CUI_EXPORT CFTypeRef
CUIFieldGetDefaultValue(CUIFieldRef field);
 
/*
 * Sets the value of a field. Note that there is no corresponding CUIFieldGetValue as
 * the provider may store a value derived from the field, or similar. When the credential
 * is submitted, the application gets the credential state by querying its attributes.
 */
CUI_EXPORT void
CUIFieldSetValue(CUIFieldRef field, CFTypeRef value);

/*
 * Set options on a field: presently this used to hide/unhide fields. CredUI will honour
 * this dynamically.
 */
CUI_EXPORT void
CUIFieldSetOptions(CUIFieldRef field, CUIFieldOptions value);

/*
 * Return options bitmask for a field.
 */
CUI_EXPORT CUIFieldOptions
CUIFieldGetOptions(CUIFieldRef field);

#ifdef __cplusplus
}
#endif

#endif /* CredUICore_CUIField_h */
