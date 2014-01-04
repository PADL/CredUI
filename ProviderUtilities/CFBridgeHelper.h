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

CF_EXPORT Boolean _CFIsObjC(CFTypeID typeID, CFTypeRef obj);
CF_EXPORT void _CFRuntimeBridgeClasses(CFTypeID cf_typeID, const char *objc_classname);
CF_EXPORT CFTypeRef _CFTryRetain(CFTypeRef cf);
CF_EXPORT Boolean _CFIsDeallocating(CFTypeRef cf);
#ifdef __cplusplus
}
#endif

#ifdef __OBJC__

@interface NSObject (CFBridgeHelper)
- (CFTypeID)_cfTypeID;
@end

#define CF_IS_SWIZZLED(obj) ([obj class] != object_getClass(obj))

#define CF_DESWIZZLE                                                            \
    Class swizzledClass = NULL;                                                 \
    @synchronized(self) {                                                       \
    if (CF_IS_SWIZZLED(self)) {                                                 \
        swizzledClass = object_getClass(self);                                  \
        object_setClass(self, [self class]);                                    \
    }                                                                           \

#define CF_RESWIZZLE                                                            \
    if (swizzledClass)                                                          \
        object_setClass(self, swizzledClass);                                   \
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

#define CF_KVO_GETTERIMPLEMENTATION(rettype, name, fn)                          \
- (rettype)name                                                                 \
{                                                                               \
    rettype ret;                                                                \
    CF_DESWIZZLE                                                                \
    ret = (rettype)fn((void *)self);                                            \
    CF_RESWIZZLE                                                                \
    return ret;                                                                 \
}

#define CF_KVO_SETTERIMPLEMENTATION(rettype, name, fn)                          \
- (void)name:(rettype)aValue                                                    \
{                                                                               \
    CF_DESWIZZLE                                                                \
    fn((void *)self, aValue);                                                   \
    CF_RESWIZZLE                                                                \
}

#define CF_KVO_SETTERIMPLEMENTATION_COPY(rettype, name, fn)                     \
- (void)name:(rettype)aValue                                                    \
{                                                                               \
    id aCopy = [aValue copy];                                                   \
    CF_DESWIZZLE                                                                \
    fn((void *)self, aCopy);                                                    \
    CF_RESWIZZLE                                                                \
    [aCopy release];                                                            \
}


#endif /* __OBJC__ */

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
    
#define CF_IS_OBJC(typeID, obj) (objc_msgSend != NULL && _CFIsObjC(typeID, (CFTypeRef)obj))

#define CF_OBJC_FUNCDISPATCHV(typeID, rettype, obj, sel, ...) \
if (CF_IS_OBJC(typeID, obj)) \
{rettype (*func)(const void *, SEL, ...) = (rettype (*)(const void *, SEL, ...))objc_msgSend; \
static SEL s = NULL; if (!s) s = sel_registerName(sel); \
return func((const void *)obj, s, ##__VA_ARGS__ );}
    
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
    
#ifdef __cplusplus
}
#endif

#endif
