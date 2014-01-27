//
//  CUIIdentityPicker+Private.m
//  CredUI
//
//  Created by Luke Howard on 28/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPicker ()
@property(nonatomic, retain, setter=_setInfo:) CUIIdentityPickerInfo *_info;
@end

@implementation CUIIdentityPicker (Private)

- (void)setModalDelegate:(id)delegate didEndSelector:(SEL)didEndSelector
{
    CUIIdentityPickerInfo *info = [[CUIIdentityPickerInfo alloc] init];
    
    info.delegate = delegate;
    info.didEndSelector = didEndSelector;
    
    self._info = info;
    
#if !__has_feature(objc_arc)
    [info release];
#endif
}

- (void)_setInfo:(CUIIdentityPickerInfo *)info
{
    if (info != _reserved[1]) {
#if !__has_feature(objc_arc)
        [_reserved[1] release];
        [info retain];
#endif
        _reserved[1] = info;
    }
}

- (CUIIdentityPickerInfo *)_info
{
    return _reserved[1];
}

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker
                  returnCode:(NSInteger)returnCode
                 contextInfo:(void *)contextInfo
{
    CUIIdentityPickerInfo *info = _reserved[1];
    NSMethodSignature *signature = [info.delegate methodSignatureForSelector:info.didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *object = (__bridge void *)self;
    
    [invocation setTarget:info.delegate];
    [invocation setSelector:info.didEndSelector];
    [invocation setArgument:&object atIndex:2];
    [invocation setArgument:&returnCode atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
}

@end
