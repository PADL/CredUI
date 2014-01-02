//
//  GSSItem.h
//  CredUI
//
//  Created by Luke Howard on 31/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#ifndef CredUI_GSSItem_h
#define CredUI_GSSItem_h

#ifdef __cplusplus
extern "C" {
#endif
    
#include <CoreFoundation/CFRuntime.h>

struct GSSItem {
    CFRuntimeBase base;
    CFMutableDictionaryRef keys;
};


/*
 * Create/Modify/Delete/Search GSS items
 *
 * Credentials needs a type, name
 */

typedef struct GSSItem *GSSItemRef;

GSSItemRef
GSSItemAdd(CFDictionaryRef attributes, CFErrorRef *error)
__attribute__((cf_returns_retained))
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemUpdate(CFDictionaryRef query, CFDictionaryRef attributesToUpdate, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemDelete(CFDictionaryRef query, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

Boolean
GSSItemDeleteItem(GSSItemRef item, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

/**
 * Will never return a zero length array, GSSItemCopyMatching() will return more then one entry or a NULL pointer.
 */

CFArrayRef
GSSItemCopyMatching(CFDictionaryRef query, CFErrorRef *error)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);



/*
 * Use a GSSItem to convert to either another type or to perform an
 * operation with the credential.
 *
 */

typedef struct __GSSOperationType const * GSSOperation;

extern const struct __GSSOperationType __kGSSOperationAcquire /* NULL, NULL|error */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationAcquire (&__kGSSOperationAcquire)

extern const struct __GSSOperationType __kGSSOperationRenewCredential
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationRenewCredential (&__kGSSOperationRenewCredential)

extern const struct __GSSOperationType __kGSSOperationGetGSSCredential /* gss_cred_it_t, NULL|error */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationGetGSSCredential (&__kGSSOperationGetGSSCredential)

extern const struct __GSSOperationType __kGSSOperationDestoryTransient /* kCFBoolean{True,False}, NULL|error */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const struct __GSSOperationType __kGSSOperationDestroyTransient /* kCFBoolean{True,False}, NULL|error */
__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);
#define kGSSOperationDestoryTransient (&__kGSSOperationDestroyTransient)
#define kGSSOperationDestroyTransient (&__kGSSOperationDestroyTransient)

extern const struct __GSSOperationType __kGSSOperationRemoveBackingCredential /* kCFBoolean{True,False}, NULL|error */
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationRemoveBackingCredential (&__kGSSOperationRemoveBackingCredential)

extern const struct __GSSOperationType __kGSSOperationChangePassword
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationChangePassword (&__kGSSOperationChangePassword)

extern const CFTypeRef kGSSOperationChangePasswordOldPassword
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
extern const CFTypeRef kGSSOperationChangePasswordNewPassword
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

extern const struct __GSSOperationType __kGSSOperationCredentialDiagnostics
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationCredentialDiagnostics (&__kGSSOperationCredentialDiagnostics)

extern const struct __GSSOperationType __kGSSOperationSetDefault
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);
#define kGSSOperationSetDefault (&__kGSSOperationSetDefault)

typedef void (^GSSItemOperationCallbackBlock)(CFTypeRef result, CFErrorRef error);

Boolean
GSSItemOperation(GSSItemRef item, GSSOperation op, CFDictionaryRef options,
                 dispatch_queue_t q, GSSItemOperationCallbackBlock fun)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

CFTypeRef
GSSItemGetValue(GSSItemRef item, CFStringRef key)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

CFTypeID
GSSItemGetTypeID(void)
__OSX_AVAILABLE_STARTING(__MAC_10_8, __IPHONE_6_0);

#ifdef __cplusplus
}
#endif

#endif
