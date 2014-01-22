//
//  CUIGSSCredCredential.cpp
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <libkern/OSAtomic.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CredUICore/CredUICore.h>

#include "CUIGSSCredCredentialProvider.h"
#include "CUIGSSCredCredential.h"
#include "CUIGSSUtilities.h"

static CFStringRef
kGSSCredCredentialProvider = CFSTR("com.padl.CredUI.Providers.GSSCredCredentialProvider");

class CUIGSSCredCredential : public CUICredentialContext {
    
public:
    
    ULONG AddRef(void) {
        return OSAtomicIncrement32Barrier(&_retainCount);
    }
    
    ULONG Release(void) {
        int32_t retainCount = OSAtomicDecrement32Barrier(&_retainCount);
        
        if (retainCount <= 0) {
            delete this;
            return 0;
        }
        return retainCount;
    }
    
    HRESULT QueryInterface(REFIID riid, void **ppv) {
        CFUUIDRef interfaceID = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, riid);
        if (CFEqual(interfaceID, kCUICredentialInterfaceID) ||
            CFEqual(interfaceID, IUnknownUUID)) {
            *ppv = this;
            AddRef();
            CFRelease(interfaceID);
            return S_OK;
        }
        CFRelease(interfaceID);
        return E_NOINTERFACE;
    }
    
    CFStringRef copyDescription(void) {
        CFStringRef desc;
        
        desc = CFStringCreateWithFormat(kCFAllocatorDefault, NULL,
                                        CFSTR("<CUIGSSCredCredential %p{name = \"%@\"}>"), this,
                                        CFDictionaryGetValue(_attributes, kCUIAttrName));
        
        return desc;
    }
    
    CFArrayRef getFields(void) {
        return _fields;
    }
    
    CFDictionaryRef getAttributes(void) {
        return _attributes;
    }

    CFStringRef copyAttributeClassForMechanisms(gss_OID_set mechanisms) {
        gss_OID oid;

        if (mechanisms == GSS_C_NO_OID_SET || !mechanisms->count)
            return NULL;

        oid = &mechanisms->elements[0];
  
        return CUICopyAttrClassForGSSOID(oid); 
    }
   
    CFDictionaryRef copyAttributesForGSSCred(gss_cred_id_t gssCred) {
        CFMutableDictionaryRef attrs;
        CFStringRef attrClass;
        OM_uint32 major, minor;
        gss_name_t name;
        gss_cred_usage_t credUsage;
        gss_OID_set mechanisms;

        if (gssCred == NULL)
            return NULL;

        attrs = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        if (attrs == NULL)
            return NULL;

        major = gss_inquire_cred(&minor, gssCred, &name, NULL, &credUsage, &mechanisms);
        if (GSS_ERROR(major) || !(credUsage & GSS_C_INITIATE)) {
            CFRelease(attrs);
            return NULL;
        }

        CFDictionarySetValue(attrs, kCUIAttrGSSCredential,          gssCred);
        CFDictionarySetValue(attrs, kCUIAttrCredentialProvider,     kGSSCredCredentialProvider);
        CFDictionarySetValue(attrs, kCUIAttrProviderFactoryID,      kGSSCredCredentialProviderFactoryID);
        CFDictionarySetValue(attrs, kCUIAttrSupportGSSCredential,   kCFBooleanTrue);
        CFDictionarySetValue(attrs, kCUIAttrCredentialStatus,       kCUICredentialReturnCredentialFinished);

        attrClass = copyAttributeClassForMechanisms(mechanisms);
        if (attrClass)
            CFDictionarySetValue(attrs, kCUIAttrClass, attrClass);

        CFUUIDRef uuid = GSSCredentialCopyUUID(gssCred);
        if (uuid) {
            CFDictionarySetValue(attrs, kCUIAttrUUID, uuid);
            CFRelease(uuid);
        }

        CFStringRef displayName = GSSNameCreateDisplayString(name);
        if (displayName) {
            CFDictionarySetValue(attrs, kCUIAttrNameDisplay, displayName);
            CFRelease(displayName);
        }
        
        gss_buffer_desc exportedName = GSS_C_EMPTY_BUFFER;
        major = gss_export_name(&minor, name, &exportedName);
        if (!GSS_ERROR(major)) {
            CFDataRef data = CUICreateDataWithGSSBuffer(exportedName);

            if (data == NULL) {
                gss_release_buffer(&minor, &exportedName);
                CFRelease(attrs);
                return NULL;
            }
     
            CFDictionarySetValue(attrs, kCUIAttrNameType, kCUIAttrNameTypeGSSExportedName);
            CFDictionarySetValue(attrs, kCUIAttrName, data);

            CFRelease(data);
            gss_release_buffer(&minor, &exportedName);
        }

        return attrs;
    }

    CFArrayRef createDefaultFields(CFStringRef defaultUsername) {
        CUIFieldRef fields[1] = { 0 };
        size_t cFields = 0;

        if (defaultUsername == NULL)
            return NULL;

        fields[cFields++] = CUIFieldCreate(kCFAllocatorDefault, kCUIFieldClassSmallText, CFSTR("Username"), defaultUsername, NULL);

        return CFArrayCreate(kCFAllocatorDefault, (const void **)fields, cFields, &kCFTypeArrayCallBacks);
    }

    Boolean initWithGSSCred(gss_cred_id_t gssCred) {
        if (gssCred == GSS_C_NO_CREDENTIAL)
            return false;

        _attributes = copyAttributesForGSSCred(gssCred);
        if (_attributes == NULL)
            return false;

        _fields = createDefaultFields((CFStringRef)CFDictionaryGetValue(_attributes, kCUIAttrNameDisplay));
        if (_fields == NULL)
            return false;

        return true;
    }

    void didBecomeSelected(Boolean *pbAutoLogin) {}
    void didBecomeDeselected(void) {}
    void didSubmit(void) {}
   
    Boolean savePersisted(CFErrorRef *) {
        return false;
    }
    
    Boolean deletePersisted(CFErrorRef *) {
        return false;
    }

    CUIGSSCredCredential() {
        _retainCount = 1;
        _fields = NULL;
        _attributes = NULL;
    }
    
private:
    int32_t _retainCount;
    CFArrayRef _fields;
    CFDictionaryRef _attributes;
    
protected:
    
    ~CUIGSSCredCredential() {
        if (_fields)
            CFRelease(_fields);
        if (_attributes)
            CFRelease(_attributes);
    }
};

CUICredentialRef
CUIGSSCredCredentialCreate(CFAllocatorRef allocator, gss_cred_id_t gssCred)
{
    CUIGSSCredCredential *context;
    CUICredentialRef cred;
    
    context = new CUIGSSCredCredential();
    if (context == NULL)
        return NULL;
    
    if (!context->initWithGSSCred(gssCred)) {
        context->Release();
        return NULL;
    }
    
    cred = CUICredentialCreate(allocator, context);
    context->Release();
    
    return cred;
}
