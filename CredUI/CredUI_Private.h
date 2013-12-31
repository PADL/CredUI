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
#import "CUICredential.h"
#import "CUICredentialTile.h"
#import "CUICredentialTileController.h"
#import "CUIIdentityPicker+Private.h"

CF_EXPORT void _CFRuntimeBridgeClasses(CFTypeID cf_typeID, const char *objc_classname);
CF_EXPORT CFTypeRef _CFTryRetain(CFTypeRef cf);
CF_EXPORT Boolean _CFIsDeallocating(CFTypeRef cf);

void NSRequestConcreteImplementation(id self, SEL _cmd, Class absClass);
