//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

/*
 * This is a thin wrapper over CUIIdentityPickerInternal, because we might want to move
 * the guts into another process sometime.
 */
@interface CUIIdentityPicker ()
@property (nonatomic, assign) NSModalResponse modalResponse;
@end

@implementation CUIIdentityPicker

#pragma mark - Initialization

- (instancetype)init
{
    return [self initWithFlags:0];
}

- (instancetype)initWithFlags:(CUIFlags)flags
{
    return [self initWithFlags:flags attributes:nil];
}

- (instancetype)initWithFlags:(CUIFlags)flags attributes:(NSDictionary *)attributes
{
    return [self initWithFlags:flags usageScenario:kCUIUsageScenarioNetwork attributes:attributes];
}

- (instancetype)initWithFlags:(CUIFlags)flags
                usageScenario:(CUIUsageScenario)usageScenario
                   attributes:(NSDictionary *)attributes
{
    if ((self = [super init]) == nil)
        return nil;

    self.modalResponse = NSModalResponseStop;
   
    _internal = [[CUIIdentityPickerInternal alloc] initWithFlags:flags
                                                   usageScenario:usageScenario
                                                      attributes:attributes];

    
    return self;
}

#pragma mark - Run Loop

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    _modalResponse = [_internal _runModal:window];

    NSMethodSignature *signature = [delegate methodSignatureForSelector:didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *object = (__bridge void *)self;
    
    [invocation setTarget:delegate];
    [invocation setSelector:didEndSelector];
    [invocation setArgument:&object atIndex:2];
    [invocation setArgument:&_modalResponse atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
}

- (NSInteger)runModal
{
    return [_internal _runModal:nil];
}

#pragma mark - Accessors

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p{targetName = \"%@\", selectedCredential = %@, flags = 0x%08x}>",
            [self.class description],
            self,
            _internal.targetDisplayName,
            _internal.selectedCredential,
            (unsigned int)self.flags];
}

#define FORWARD_PROPERTY(type, setter, getter)                          \
                                                                        \
- (type)getter                                                          \
{                                                                       \
    return _internal.getter;                                            \
}                                                                       \
                                                                        \
- (void)setter:(type)arg                                                \
{                                                                       \
    _internal.getter = arg;                                             \
}                                                                        

FORWARD_PROPERTY(NSString *,            setTitle,               title)
FORWARD_PROPERTY(NSString *,            setMessage,             message)
FORWARD_PROPERTY(NSDictionary *,        setAttributes,          attributes)
FORWARD_PROPERTY(NSError *,             setAuthError,           authError)
FORWARD_PROPERTY(BOOL,                  setPersist,             persist)
FORWARD_PROPERTY(const void *,          setContext,             context)
FORWARD_PROPERTY(id,                    setTargetName,          targetName)

/* readonly properties */

- (CUIFlags)flags
{
    return _internal.flags;
}

- (CUICredential *)selectedCredential
{
    if (_modalResponse != NSModalResponseOK)
        return nil;
        
    return _internal.selectedCredential;
}

- (NSError *)lastError
{
    return _internal.lastError;
}

@end
