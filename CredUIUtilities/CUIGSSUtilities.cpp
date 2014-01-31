//
//  CUIGSSUtilities.cpp
//  CredUI
//
//  Created by Luke Howard on 22/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <CredUICore/CUIAttributes.h>
#include <GSS/GSS.h>

#include "CUIGSSUtilities.h"

extern "C" {
    
typedef struct heim_oid {
    size_t length;
    unsigned *components;
} heim_oid;

extern int
der_parse_heim_oid (const char *str, const char *sep, heim_oid *data);

extern int
der_put_oid (unsigned char *p, size_t len,
             const heim_oid *data, size_t *size);

extern void
der_free_oid (heim_oid *k);

}

Boolean
CUICreateGSSBufferWithString(CFStringRef cfString, gss_buffer_desc &buffer)
{
    OM_uint32 tmpMinor;
    
    if (cfString == NULL || CFGetTypeID(cfString) != CFStringGetTypeID())
        return false;
    
    buffer.length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(cfString),
                                                      kCFStringEncodingUTF8);
    buffer.value = malloc(buffer.length + 1);
    if (buffer.value == NULL)
        return false;
    
    if (!CFStringGetCString(cfString, (char *)buffer.value,
                            buffer.length, kCFStringEncodingUTF8)) {
        gss_release_buffer(&tmpMinor, &buffer);
        return false;
    }
    
    return true;
}

CFStringRef
CUICreateStringWithGSSBuffer(const gss_buffer_desc &buffer)
{
    return CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)buffer.value, buffer.length, kCFStringEncodingUTF8, false);
}

Boolean
CUICreateGSSBufferWithData(CFDataRef cfData, gss_buffer_desc &buffer)
{
    if (cfData == NULL || CFGetTypeID(cfData) != CFDataGetTypeID())
        return false;
   
    buffer.length = CFDataGetLength(cfData); 
    buffer.value = malloc(buffer.length);
    if (buffer.value == NULL)
        return false;
  
    CFDataGetBytes(cfData, CFRangeMake(0, buffer.length), (UInt8 *)buffer.value); 

    return true;
}

CFDataRef
CUICreateDataWithGSSBuffer(const gss_buffer_desc &buffer)
{
    return CFDataCreate(kCFAllocatorDefault, (const UInt8 *)buffer.value, buffer.length);
}

CFStringRef
CUICreateStringWithGSSOID(gss_OID oid)
{
    gss_buffer_desc buffer = GSS_C_EMPTY_BUFFER;
    OM_uint32 minor;
    CFStringRef string = NULL;
    
    if (!GSS_ERROR(gss_oid_to_str(&minor, oid, &buffer))) {
        string = CUICreateStringWithGSSBuffer(buffer);
        gss_release_buffer(&minor, &buffer);
    }
    
    return string;
}

Boolean
CUICreateGSSOIDWithString(CFStringRef cfString, gss_OID_desc &oidBuf)
{
    OM_uint32 tmpMinor;
    gss_buffer_desc stringBuf = GSS_C_EMPTY_BUFFER;
    char mechbuf[64];
    size_t mech_len;
    heim_oid heimOid;
    int ret;
    
    if (!CUICreateGSSBufferWithString(cfString, stringBuf))
        return false;
    
    if (der_parse_heim_oid((char *)stringBuf.value, " .", &heimOid)) {
        gss_release_buffer(&tmpMinor, &stringBuf);
        return false;
    }
    
    gss_release_buffer(&tmpMinor, &stringBuf);
    
    ret = der_put_oid ((unsigned char *)mechbuf + sizeof(mechbuf) - 1,
                       sizeof(mechbuf),
                       &heimOid,
                       &mech_len);
    if (ret) {
        der_free_oid(&heimOid);
        return false;
    }
    
    oidBuf.length = (OM_uint32)mech_len;
    oidBuf.elements = malloc(oidBuf.length);
    if (oidBuf.elements == NULL) {
        der_free_oid(&heimOid);
        return false;
    }
    
    memcpy(oidBuf.elements, mechbuf + sizeof(mechbuf) - mech_len, mech_len);
    
    der_free_oid(&heimOid);
    
    return true;
}

gss_OID
CUICopyGSSOIDForAttrClass(CFStringRef attrClass, gss_OID_desc &oidBuf)
{
    gss_OID oid = GSS_C_NO_OID;

    if (attrClass == NULL)
        return GSS_C_NO_OID;
    
    if (CFEqual(attrClass, kCUIAttrClassKerberos))
        oid = GSS_KRB5_MECHANISM;
    else if (CFEqual(attrClass, kCUIAttrClassNTLM))
        oid = GSS_NTLM_MECHANISM;
    else if (CFEqual(attrClass, kCUIAttrClassIAKerb))
        oid = GSS_IAKERB_MECHANISM;
    else
        if (CUICreateGSSOIDWithString(attrClass, oidBuf))
            oid = &oidBuf;
        
    return oid;
}

/*
 * XXX This unfortunately is very brittle. It would be much simpler if
 * the class strings were OIDs.
 */
CFStringRef
CUIAttrClassForMech(CFStringRef errMechName, CFStringRef errMechOid)
{
    if (errMechName) {
        if (CFEqual(errMechName, CFSTR("krb5")))
            return kCUIAttrClassKerberos;
        else if (CFEqual(errMechName, CFSTR("ntlm")))
            return kCUIAttrClassNTLM;
        else if (CFEqual(errMechName, CFSTR("iakerb")))
            return kCUIAttrClassIAKerb;
    }
    
    return errMechOid;
}

CFStringRef
CUICopyAttrClassForGSSOID(gss_OID oid)
{
    CFStringRef attrClass = NULL;
    
    if (oid == GSS_C_NO_OID)
        return NULL;
    
    if (gss_oid_equal(oid, GSS_KRB5_MECHANISM))
        attrClass = kCUIAttrClassKerberos;
    else if (gss_oid_equal(oid, GSS_NTLM_MECHANISM))
        attrClass = kCUIAttrClassNTLM;
    else if (gss_oid_equal(oid, GSS_IAKERB_MECHANISM))
        attrClass = kCUIAttrClassIAKerb;
    else
        attrClass = CUICreateStringWithGSSOID(oid);

    return attrClass;
}

gss_name_t
CUICopyGSSNameForAttributes(CFDictionaryRef attributes)
{
    gss_name_t name = GSS_C_NO_NAME;
    gss_const_OID oid = GSS_C_NO_OID;
    
    CFTypeRef type = CFDictionaryGetValue(attributes, kCUIAttrNameType);
    CFTypeRef value = CFDictionaryGetValue(attributes, kCUIAttrName);
    
    if (type == NULL || CFEqual(type, kCUIAttrNameTypeGSSUsername))
        oid = GSS_C_NT_USER_NAME;
    else if (CFEqual(type, kCUIAttrNameTypeGSSHostBasedService))
        oid = GSS_C_NT_HOSTBASED_SERVICE;
    else if (CFEqual(type, kCUIAttrNameTypeGSSExportedName))
        oid = GSS_C_NT_EXPORT_NAME;
    
    if (oid != GSS_C_NO_OID && value != NULL)
        name = GSSCreateName(value, oid, NULL);
    
    return name;
}

CFStringRef kGSSMajorErrorCode = CFSTR("kGSSMajorError");
CFStringRef kGSSMinorErrorCode = CFSTR("kGSSMinorError");
CFStringRef kGSSMechanismOID   = CFSTR("kGSSMechanismOID");
CFStringRef kGSSMechanism      = CFSTR("kGSSMechanism");

CFErrorRef
CUIGSSErrorCreate(OM_uint32 major, OM_uint32 minor, gss_OID mechanism)
{
    if (major == GSS_S_COMPLETE)
        return NULL;

    CFErrorRef error;
    CFDictionaryRef userInfo = NULL; // XXX
    
    error = CFErrorCreate(kCFAllocatorDefault, CFSTR("org.h5l.GSS"), major, userInfo);

    return error;
}

void
CUIGSSErrorComplete(void (^completionHandler)(CFErrorRef), OM_uint32 major, OM_uint32 minor, gss_OID mechanism)
{
    CFErrorRef errorRef = CUIGSSErrorCreate(major, minor, mechanism);

    completionHandler(errorRef);

    if (errorRef)
        CFRelease(errorRef);
}

Boolean
CUIIsGSSError(CFErrorRef error)
{
    CFStringRef domain = error ? CFErrorGetDomain(error) : NULL;
    
    return domain && CFEqual(domain, CFSTR("org.h5l.GSS"));
}
