//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

static NSString *const CUIIdentityPickerUseViewBridge = @"CUIIdentityPickerUseViewBridge";

/*
 * This is a thin wrapper over CUIIdentityPickerInternal, because we might want to move
 * the guts into another process sometime. And it lets us hide instance variables on
 * the old Objective-C runtime.
 */

@implementation CUIIdentityPicker

#pragma mark - User Defaults

+ (void)initialize
{
    NSDictionary *defaults = [NSDictionary dictionaryWithObjectsAndKeys:@YES, CUIIdentityPickerUseViewBridge, nil];

    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];
}

+ (BOOL)useViewBridgeIdentityPicker
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];

    return [standardUserDefaults boolForKey:CUIIdentityPickerUseViewBridge];
}

#pragma mark - Initialization

+ allocWithZone:(NSZone *)zone
{
    if ([self useViewBridgeIdentityPicker] && [self isEqual:[CUIIdentityPicker class]]) {
        return [CUIVBIdentityPicker allocWithZone:zone];
    } else {
        return [super allocWithZone:zone];
    }
}

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
    return [self initWithUsageScenario:kCUIUsageScenarioNetwork];
}

- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
{
    return [self initWithUsageScenario:usageScenario attributes:nil];
}

- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attributes
{
    return [self initWithUsageScenario:usageScenario attributes:attributes flags:0];
}

- (instancetype)initWithUsageScenario:(CUIUsageScenario)usageScenario
                           attributes:(NSDictionary *)attributes
                                flags:(CUIFlags)flags
{
    if ((self = [super init]) == nil)
        return nil;

    CUIIdentityPickerInternal *identityPicker = [[CUIIdentityPickerInternal alloc] init];

    if (![identityPicker configureForUsageScenario:usageScenario flags:flags]) {
#if !__has_feature(objc_arc)
        [identityPicker release];
        [self release];
#endif
        return nil;
    }

    if (attributes)
        identityPicker.attributes = attributes;

    _reserved[0] = identityPicker;
 
    return self;
}

#pragma mark - Run Loop

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
    [self setModalDelegate:delegate didEndSelector:didEndSelector];

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
