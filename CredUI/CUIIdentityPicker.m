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
@property (nonatomic, retain) id delegate;
@property (nonatomic) SEL didEndSelector;
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

    _internal = [[CUIIdentityPickerInternal alloc] initWithFlags:flags
                                                   usageScenario:usageScenario
                                                      attributes:attributes];

    
    return self;
}

#pragma mark - Run Loop

- (void)identityPickerDidEnd:(CUIIdentityPicker *)identityPicker returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    NSMethodSignature *signature = [self.delegate methodSignatureForSelector:self.didEndSelector];
    NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
    void *object = (__bridge void *)self;

    [invocation setTarget:self.delegate];
    [invocation setSelector:self.didEndSelector];
    [invocation setArgument:&object atIndex:2];
    [invocation setArgument:&returnCode atIndex:3];
    [invocation setArgument:&contextInfo atIndex:4];
    [invocation invoke];
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    self.delegate = delegate;
    self.didEndSelector = didEndSelector;

    return [_internal runModalForWindow:window
                          modalDelegate:self
                         didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                            contextInfo:contextInfo];
}

- (NSInteger)runModal
{
    return [_internal runModal];
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
    return _internal.selectedCredential;
}

- (NSError *)lastError
{
    return _internal.lastError;
}

@end
