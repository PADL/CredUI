//
//  CUIGSSItemCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 6/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include "CUIGSSItemCredential.h"

CFDictionaryRef CUIGSSItemCredential::copyItemAttributes(void)
{
    return CUICreateGSSItemAttributesFromCUIAttributes(getAttributes());    
}

Boolean CUIGSSItemCredential::savePersisted(CFErrorRef *error)
{
    CFDictionaryRef gssItemAttributes = NULL;
    Boolean ret;
    
    /*
     * We update any existing GSS items on behalf of the credential provider, however
     * adding any new ones must be done by the credential provider themselves.
     */
    ret = CUICredentialSavePersisted(_credential, error);
    if (!ret)
        return ret;
    
    if (_item) {
        gssItemAttributes = copyItemAttributes();
        if (gssItemAttributes == NULL)
            return false;

        /*
         * There's a bug in Heimdal, if the keychain item already exists it will
         * create a new one, not update it, which will fail. So if we have a password
         * to store, trash the item and retry.
         */
#ifdef BUGGY_GSSITEM_UPDATE
        if (CFDictionaryGetValue(gssItemAttributes, kGSSAttrCredentialPassword)) {
            GSSItemDeleteItem(_item, NULL);
            CFRelease(_item);
            _item = GSSItemAdd(gssItemAttributes, error);
        } else
#endif
            ret = GSSItemUpdate(_item->keys, gssItemAttributes, error);
    }

    if (gssItemAttributes)
        CFRelease(gssItemAttributes);

    return ret;
}

Boolean CUIGSSItemCredential::deletePersisted(CFErrorRef *error)
{
    CFDictionaryRef gssItemAttributes;
    Boolean ret;
    
    ret = CUICredentialDeletePersisted(_credential, error);
    if (!ret)
        return ret;
    
    gssItemAttributes = copyItemAttributes();
    if (gssItemAttributes == NULL)
        return false;
    
    ret = GSSItemDelete(gssItemAttributes, error);
    CFRelease(gssItemAttributes);
    
    return ret;
}

