//
//  CUIGSSUtilities.h
//  CredUI
//
//  Created by Luke Howard on 22/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_GSSUtilities_h
#define CredUI_GSSUtilities_h

Boolean
CUICreateGSSBufferWithString(CFStringRef cfString, gss_buffer_desc &buffer);

CFStringRef
CUICreateStringWithGSSBuffer(const gss_buffer_desc &buffer);

Boolean
CUICreateGSSOIDWithString(OM_uint32 *minor, CFStringRef cfString, gss_OID_desc &oidBuf);

CFStringRef
CUICreateStringWithGSSOID(gss_OID oid);

gss_OID
CUICopyGSSOIDForAttrClass(CFStringRef attrClass, gss_OID_desc &oidBuf);

CFStringRef
CUICopyAttrClassForGSSOID(gss_OID oid);

CFDataRef
CUICreateDataWithGSSBuffer(const gss_buffer_desc &buffer);

Boolean
CUICreateGSSBufferWithData(CFDataRef cfData, gss_buffer_desc &buffer);

#endif /* CredUI_GSSUtilities_h */
