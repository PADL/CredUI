//
//  CredUI_Private.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>
#import <CoreFoundation/CFPlugInCOM.h>
#import <CoreFoundation/CFBridgingPriv.h>

#import <CredUI/CredUI.h>
#import <CredUI/CUICredential+CBIdentity.h>

#import <CredUICore/CredUICore.h>

#import "CUIField.h"
#import "CUIField+UIFactory.h"

#import "CUICredential+Private.h"

#import "CUICredentialTile.h"
#import "CUICredentialTileController.h"

#import "CUIIdentityPickerInternal.h"

#import "NSBundle+CredUI.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef CUI_EXPORT
#define CUI_EXPORT  __attribute__((visibility("default")))

extern void
NSRequestConcreteImplementation(id self, SEL _cmd, Class absClass);

CUI_EXPORT Boolean
_CUIPromptForCredentials(CFTypeRef targetName,
                         CFTypeRef contextHandle,
                         CUICredUIContext *uiContext,
                         CFErrorRef authError,
                         CFDictionaryRef inCredAttributes,
                         CUICredentialRef *outCredential,
                         Boolean *pfSave,
                         CUIFlags flags,
                         CFErrorRef *error);

#ifdef __cplusplus
}
#endif
