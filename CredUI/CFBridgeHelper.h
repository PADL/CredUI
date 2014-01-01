//
//  CFBridgeHelper.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_CFBridgeHelper_h
#define CredUI_CFBridgeHelper_h

#define CF_CLASSIMPLEMENTATION(ClassName)                                       \
- (id)retain                                                                    \
{                                                                               \
    return CFRetain((CFTypeRef)self);                                           \
}                                                                               \
                                                                                \
- (oneway void)release                                                          \
{                                                                               \
    CFRelease((CFTypeRef)self);                                                 \
}                                                                               \
                                                                                \
- (NSUInteger)retainCount                                                       \
{                                                                               \
    return CFGetRetainCount((CFTypeRef)self);                                   \
}                                                                               \
                                                                                \
- (BOOL)isEqual:(id)anObject                                                    \
{                                                                               \
    return (BOOL)CFEqual((CFTypeRef)self, (CFTypeRef)anObject);                 \
}                                                                               \
                                                                                \
- (NSUInteger)hash                                                              \
{                                                                               \
    return CFHash((CFTypeRef)self);                                             \
}                                                                               \
                                                                                \
- (BOOL)allowsWeakReference                                                     \
{                                                                               \
    return !_CFIsDeallocating(self);                                            \
}                                                                               \
                                                                                \
- (BOOL)retainWeakReference                                                     \
{                                                                               \
    return _CFTryRetain(self) != nil;                                           \
}                                                                               \

                                                                                
#endif
