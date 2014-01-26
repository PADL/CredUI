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
