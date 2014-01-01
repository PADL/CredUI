/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
/*	CFInternal.h
 Copyright (c) 1998-2003, Apple, Inc. All rights reserved.
 */

#ifndef CredUI_CFObjCHelper_h
#define CredUI_CFObjCHelper_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include <objc/message.h>

OBJC_EXPORT id objc_msgSend(id self, SEL op, ...) __attribute__((weak_import));
    
OBJC_EXPORT Boolean _CFIsObjC(CFTypeID typeID, CFTypeRef obj);

#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel) \
    if (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj)) \
    {rettype (*func)(const void *, SEL) = (rettype (*)(const void *, SEL))objc_msgSend; \
    static SEL s = NULL; if (!s) s = sel_registerName(sel); \
    return func((const void *)obj, s);}
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1) \
if (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj)) \
    {rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
    static SEL s = NULL; if (!s) s = sel_registerName(sel); \
    return func((const void *)obj, s, (a1));}
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2) \
if (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj)) \
    {rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
    static SEL s = NULL; if (!s) s = sel_registerName(sel); \
    return func((const void *)obj, s, (a1), (a2));}
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3) \
if (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj)) \
    {rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
    static SEL s = NULL; if (!s) s = sel_registerName(sel); \
    return func((const void *)obj, s, (a1), (a2), (a3));}
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4) \
if (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj)) \
    {rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
    static SEL s = NULL; if (!s) s = sel_registerName(sel); \
    return func((const void *)obj, s, (a1), (a2), (a3), (a4));}

#ifdef __cplusplus
}
#endif

#endif
