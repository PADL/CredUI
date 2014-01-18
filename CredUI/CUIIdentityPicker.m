//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInfo : NSObject
{
    id _delegate;
    SEL _didEndSelector;
}
@property(nonatomic, retain) id delegate;
@property(nonatomic, assign) SEL didEndSelector;
@end

@implementation CUIIdentityPickerInfo
@synthesize delegate = _delegate;
@synthesize didEndSelector = _didEndSelector;
@end

/*
 * This is a thin wrapper over CUIIdentityPickerInternal, because we might want to move
 * the guts into another process sometime. And it lets us hide instance variables on
 * the old Objective-C runtime.
 */

@implementation CUIIdentityPicker

#pragma mark - Initialization

- (void)dealloc
{
#if !__has_feature(objc_arc)
    [_reserved[0] release];
    [_reserved[1] release];

    [super dealloc];
#endif
}

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

    _reserved[0] = [[CUIIdentityPickerInternal alloc] initWithUsageScenario:usageScenario
                                                                      flags:flags
                                                                 attributes:attributes];
    
    return self;
}

#pragma mark - Run Loop

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

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    CUIIdentityPickerInfo *info = [[CUIIdentityPickerInfo alloc] init];

    info.delegate = delegate;
    info.didEndSelector = didEndSelector;

#if !__has_feature(objc_arc)
    [_reserved[1] release];
#endif
    _reserved[1] = info;

    [_reserved[0] runModalForWindow:window
                      modalDelegate:self
                     didEndSelector:@selector(identityPickerDidEnd:returnCode:contextInfo:)
                        contextInfo:contextInfo];
}

- (NSInteger)runModal
{
    return [_reserved[0] runModal];
}

#pragma mark - Accessors

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p{targetName = \"%@\", selectedCredential = %@, flags = 0x%08x}>",
            [self.class description],
            self,
            [_reserved[0] targetDisplayName],
            [_reserved[0] selectedCredential],
            (unsigned int)self.flags];
}

#define FORWARD_PROPERTY_RO(type, getter)                               \
- (type)getter                                                          \
{                                                                       \
    return ((CUIIdentityPickerInternal *)_reserved[0]).getter;          \
}                                                                       \

#define FORWARD_PROPERTY(type, setter, getter)                          \
                                                                        \
FORWARD_PROPERTY_RO(type, getter)                                       \
                                                                        \
- (void)setter:(type)arg                                                \
{                                                                       \
    ((CUIIdentityPickerInternal *)_reserved[0]).getter = arg;           \
}                                                                        

FORWARD_PROPERTY(NSString *,            setTitle,               title)
FORWARD_PROPERTY(NSString *,            setMessage,             message)
FORWARD_PROPERTY(NSDictionary *,        setAttributes,          attributes)
FORWARD_PROPERTY(NSError *,             setAuthError,           authError)
FORWARD_PROPERTY(BOOL,                  setPersist,             persist)
FORWARD_PROPERTY(const void *,          setContext,             context)
FORWARD_PROPERTY(id,                    setTargetName,          targetName)

FORWARD_PROPERTY_RO(CUICredential *,    selectedCredential)
FORWARD_PROPERTY_RO(NSError *,          lastError)
FORWARD_PROPERTY_RO(CUIFlags,           flags)

@end
