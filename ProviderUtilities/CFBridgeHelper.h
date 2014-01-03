//
//  CFBridgeHelper.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CFBridgeHelper_h
#define CFBridgeHelper_h

#include <objc/message.h>
#include <objc/objc-sync.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif
OBJC_EXPORT id objc_msgSend(id self, SEL op, ...) __attribute__((weak_import));
OBJC_EXPORT Boolean _CFIsObjC(CFTypeID typeID, CFTypeRef obj);
#ifdef __cplusplus
}
#endif

#ifdef __OBJC__

@interface NSObject (CFBridgeHelper)
- (CFTypeID)_cfTypeID;
@end

#define CF_IS_SWIZZLED(obj) ([obj class] != object_getClass(obj))

#define CF_DESWIZZLE                                                            \
    Class __swiz = NULL;                                                        \
    @synchronized(self) {                                                       \
    if (CF_IS_SWIZZLED(self)) {                                                 \
        __swiz = object_getClass(self);                                         \
        object_setClass(self, [self class]);                                    \
    }                                                                           \

#define CF_RESWIZZLE                                                            \
    if (__swiz)                                                                 \
        object_setClass(self, __swiz);                                          \
    }                                                                           \

#define CF_CLASSIMPLEMENTATION(ClassName)                                       \
- (id)retain                                                                    \
{                                                                               \
    id obj = nil;                                                               \
    CF_DESWIZZLE                                                                \
    obj = CFRetain((CFTypeRef)self);                                            \
    CF_RESWIZZLE                                                                \
    return obj;                                                                 \
}                                                                               \
                                                                                \
- (oneway void)release                                                          \
{                                                                               \
    CF_DESWIZZLE                                                                \
    CFRelease((CFTypeRef)self);                                                 \
    CF_RESWIZZLE                                                                \
}                                                                               \
                                                                                \
- (NSUInteger)retainCount                                                       \
{                                                                               \
    NSUInteger rc;                                                              \
    CF_DESWIZZLE                                                                \
    rc = CFGetRetainCount((CFTypeRef)self);                                     \
    CF_RESWIZZLE                                                                \
    return rc;                                                                  \
}                                                                               \
                                                                                \
- (BOOL)isEqual:(id)anObject                                                    \
{                                                                               \
    BOOL ret;                                                                   \
    if (anObject == nil)                                                        \
        return NO;                                                              \
    CF_DESWIZZLE                                                                \
    ret = CFEqual((CFTypeRef)self, (CFTypeRef)anObject);                        \
    CF_RESWIZZLE                                                                \
    return ret;                                                                 \
}                                                                               \
                                                                                \
- (NSUInteger)hash                                                              \
{                                                                               \
    NSUInteger hash;                                                            \
    CF_DESWIZZLE                                                                \
    hash = CFHash((CFTypeRef)self);                                             \
    CF_RESWIZZLE                                                                \
    return hash;                                                                \
}                                                                               \
                                                                                \
- (BOOL)allowsWeakReference                                                     \
{                                                                               \
    return ![self _isDeallocating];                                             \
}                                                                               \
                                                                                \
- (BOOL)retainWeakReference                                                     \
{                                                                               \
    return [self _tryRetain];                                                   \
}                                                                               \
                                                                                \
- (BOOL)_isDeallocating                                                         \
{                                                                               \
    BOOL ret;                                                                   \
    CF_DESWIZZLE                                                                \
    ret = _CFIsDeallocating((CFTypeRef)self);                                   \
    CF_RESWIZZLE                                                                \
    return ret;                                                                 \
}                                                                               \
                                                                                \
- (BOOL)_tryRetain                                                              \
{                                                                               \
    BOOL ret;                                                                   \
    CF_DESWIZZLE                                                                \
    ret = _CFTryRetain((CFTypeRef)self) != NULL;                                \
    CF_RESWIZZLE                                                                \
    return ret;                                                                 \
}                                                                               \
                                                                                \
- (NSString *)description                                                       \
{                                                                               \
    NSString *desc;                                                             \
    CF_DESWIZZLE                                                                \
    desc = [NSMakeCollectable(CFCopyDescription((CFTypeRef)self)) autorelease]; \
    CF_RESWIZZLE                                                                \
    return desc;                                                                \
}                                                                               \

#else

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

#ifdef __cplusplus
extern "C" {
#endif

static inline Boolean
__CFIsReallyObjC(CFTypeID typeID, CFTypeRef obj)
{
    if (objc_msgSend == NULL || !_CFIsObjC(typeID, obj))
        return 0;
    
    static SEL s = NULL;
    Class (*func)(const void *, SEL) = (Class (*)(const void *, SEL))objc_msgSend;
    
    if (!s)
        s = sel_registerName("class");
    
    assert(s);
    
    Class swizzledClass = object_getClass((id)obj);
    Class reportedClass = func(obj, s);
    
    assert(swizzledClass);
    assert(reportedClass);
    
    if (swizzledClass == reportedClass)
        return 1; /* is non-swizzled ObjC object */
    
    /* OK, now the tricky part */
    objc_sync_enter((id)obj);
    object_setClass((id)obj, reportedClass);
    Boolean ret = _CFIsObjC(typeID, obj);
    object_setClass((id)obj, swizzledClass);
    objc_sync_exit((id)obj);
    
    return ret;
}

#define CF_OBJC_FUNCDISPATCH0(typeID, rettype, obj, sel) \
if (objc_msgSend != NULL && __CFIsReallyObjC(typeID, (CFTypeRef)obj)) \
{rettype (*func)(const void *, SEL) = (rettype (*)(const void *, SEL))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s);}
#define CF_OBJC_FUNCDISPATCH1(typeID, rettype, obj, sel, a1) \
if (objc_msgSend != NULL && __CFIsReallyObjC(typeID, (CFTypeRef)obj)) \
{rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s, (a1));}
#define CF_OBJC_FUNCDISPATCH2(typeID, rettype, obj, sel, a1, a2) \
if (objc_msgSend != NULL && __CFIsReallyObjC(typeID, (CFTypeRef)obj)) \
{rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s, (a1), (a2));}
#define CF_OBJC_FUNCDISPATCH3(typeID, rettype, obj, sel, a1, a2, a3) \
if (objc_msgSend != NULL && __CFIsReallyObjC(typeID, (CFTypeRef)obj)) \
{rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s, (a1), (a2), (a3));}
#define CF_OBJC_FUNCDISPATCH4(typeID, rettype, obj, sel, a1, a2, a3, a4) \
if (objc_msgSend != NULL && __CFIsReallyObjC(typeID, (CFTypeRef)obj)) \
{rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s, (a1), (a2), (a3), (a4));}
    
#define CF_OBJC_KVO_WILLCHANGE(obj, key) \
if (objc_msgSend != NULL) \
{void (*func)(const void *, SEL, CFStringRef) = (void (*)(const void *, SEL, CFStringRef))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName("willChangeValueForKey:"); \
func((const void *)obj, s, (key));}
#define CF_OBJC_KVO_DIDCHANGE(obj, key) \
if (objc_msgSend != NULL) \
{void (*func)(const void *, SEL, CFStringRef) = (void (*)(const void *, SEL, CFStringRef))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName("didChangeValueForKey:"); \
func((const void *)obj, s, (key));}
    
#endif /* __OBJC__ */
    
#ifdef __cplusplus
}
#endif

#endif
