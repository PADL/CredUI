//
//  CredUI_Private.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CoreFoundation/CoreFoundation.h>
#import <CoreFoundation/CFPlugInCOM.h>

#import <GSS/GSS.h>

#import <CredUI/CredUI.h>
#import <CredUI/CUICredential+GSS.h>
#import <CredUI/GSSPromptForCredentials.h>

#import <CredUICore/CredUICore.h>
#import <CredUICore/CredUICore_Private.h>

#import "CUIField.h"
#import "CUIField+UIFactory.h"

#import "CUICredential+Private.h"

#import "CUICredentialTile.h"
#import "CUICredentialTileController.h"

#import "CUIIdentityPickerInternal.h"
#import "CUIIdentityPickerInternal+UIFactory.h"

extern Boolean
_CUIPromptForCredentials(CFTypeRef targetName,
                         CFTypeRef gssContextHandle,
                         CUICredUIContext *uiContext,
                         CFErrorRef authError,
                         CFDictionaryRef inCredAttributes,
                         CUICredentialRef *outCredential,
                         Boolean *pfSave,
                         CUIFlags flags,
                         CFErrorRef *error);

extern void NSRequestConcreteImplementation(id self, SEL _cmd, Class absClass);
