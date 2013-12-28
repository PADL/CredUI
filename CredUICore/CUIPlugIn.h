//
//  CUIProvider.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__CUIProvider__
#define __CredUI__CUIProvider__

#include <CoreFoundation/CFPlugInCOM.h>

// BFA3619B-1A12-4DBA-801F-33B0874DD76F
#define kCUIPlugInTypeID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xBF, 0xA3, 0x61, 0x9B, 0x1A, 0x12, 0x4D, 0xBA, 0x80, 0x1F, 0x33, 0xB0, 0x87, 0x4D, 0xD7, 0x6F)

// F7356A4B-91A7-4455-AF3A-175D27449C9E
#define kCUIPlugInInterfaceID CFUUIDGetConstantUUIDWithBytes(kCFAllocatorSystemDefault, 0xF7, 0x35, 0x6A, 0x4B, 0x91, 0xA7, 0x44, 0x55, 0xAF, 0x3A, 0x17, 0x5D, 0x27, 0x44, 0x9C, 0x9E)

#ifdef __cplusplus
class CUIPlugIn : IUnknown {
    virtual HRESULT setUsageScenario(CUIUsageScenario scenario, CUIUsageFlags flags);
    virtual CFArrayRef getCredentials(void); // array of CUICredentialRef
};
#else
typedef struct CUIPlugInVTbl {
    IUNKNOWN_C_GUTS;
    HRESULT (STDMETHODCALLTYPE *setUsageScenario)(void *thisPointer, CUIUsageScenario scenario, CUIUsageFlags flags);
    CFArrayRef (STDMETHODCALLTYPE *getCredentials)(void *thisPointer);
} CUIPlugInVTbl;
#endif /* defined(__cplusplus) */

#endif /* defined(__CredUI__CUIProvider__) */