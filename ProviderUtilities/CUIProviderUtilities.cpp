//
//  CUIProviderUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <GSS/GSS.h>

#include <CredUICore/CredUICore.h>

#include "CUIProviderUtilities.h"
#include "GSSItem.h"

CUIAttributeSource
CUIGetAttributeSource(CFDictionaryRef attributes)
{
    CFUUIDRef persistenceFactoryID = (CFUUIDRef)CFDictionaryGetValue(attributes, kCUIAttrPersistenceFactoryID);

    if (persistenceFactoryID == NULL)
        return kCUIAttributeSourceUser;

    if (CFEqual(persistenceFactoryID, kGSSItemCredentialProviderFactoryID))
        return kCUIAttributeSourceGSSItem;
    else if (CFEqual(persistenceFactoryID, kKeychainCredentialProviderFactoryID))
        return kCUIAttributeSourceKeychain;
    else
        return kCUIAttributeSourceUnknown;
}

CFStringRef
CUIGetDefaultUsername(CFDictionaryRef attributes)
{
    CFStringRef defaultUsername;

    defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameDisplay);
    if (defaultUsername == NULL) {
        CFStringRef nameType = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrNameType);
        
        if (nameType && CFEqual(nameType, kCUIAttrNameTypeGSSUsername))
            defaultUsername = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrName);
    }
    
    return defaultUsername;
}

Boolean
CUIShouldEnumerateForClass(CFDictionaryRef attributes, CFStringRef mechClass)
{
    CFStringRef attrClass = (CFStringRef)CFDictionaryGetValue(attributes, kCUIAttrClass);
    
    if (attrClass == NULL)
        return TRUE;
    
    return CFEqual(attrClass, mechClass);
}

Boolean
CUIShouldEnumerateForPasswordClass(CFDictionaryRef attributes)
{
    return CUIShouldEnumerateForClass(attributes, kCUIAttrClassKerberos) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassNTLM) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassIAKerb) ||
    CUIShouldEnumerateForClass(attributes, kCUIAttrClassGeneric);
}

CUICredentialPersistence *
CUICreatePersistenceForSource(CUIControllerRef controller, CUIAttributeSource source)
{
    CUICredentialPersistence *persistence = NULL;
    CFUUIDRef factoryID = NULL;
    
    switch (source) {
        case kCUIAttributeSourceGSSItem:
            factoryID = kGSSItemCredentialProviderFactoryID;
            break;
        case kCUIAttributeSourceKeychain:
            factoryID = kKeychainCredentialProviderFactoryID;
            break;
        default:
            break;
    }
    
    if (factoryID)
        persistence = __CUIControllerCreatePersistenceForFactoryID(controller, factoryID);
    
    return persistence;
}
