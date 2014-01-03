//
//  CredUI_Private.h
//  CredUI
//
//  Created by Luke Howard on 28/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import <CoreFoundation/CFPlugInCOM.h>

#import <CredUI/CredUI.h>
#import <CredUI/GSSPromptForCredentials.h>

#import <CredUICore/CredUICore.h>
#import <CredUICore/CredUICore_Private.h>

#import "CUIField.h"
#import "CUICredential+Private.h"

#import "CUICredentialTile.h"
#import "CUICredentialTileController.h"

#import "CUIIdentityPickerInternal.h"
#import "CUIIdentityPickerInternal+UIFactory.h"

void NSRequestConcreteImplementation(id self, SEL _cmd, Class absClass);

Boolean
__CUIPromptForCredentials(CFTypeRef targetName,
                          CFTypeRef gssContextHandle,
                          CUICredUIContext *uiContext,
                          CFErrorRef authError,
                          CFDictionaryRef inCredAttributes,
                          CFDictionaryRef *outCredAttributes,
                          Boolean *pfSave,
                          CUIFlags flags,
                          CUIAttributeClass attrClass);

