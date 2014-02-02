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

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow
                   modalDelegate:(id)delegate
                  didEndSelector:(SEL)didEndSelector
                     contextInfo:(void *)contextInfo
{
    [self beginSheetModalForWindow:sheetWindow
                 completionHandler:^(NSModalResponse modalResponse) {
        NSMethodSignature *signature = [delegate methodSignatureForSelector:didEndSelector];
        NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:signature];
        const void *object = (__bridge const void *)self;

        [invocation setTarget:delegate];
        [invocation setSelector:didEndSelector];
        [invocation setArgument:&object atIndex:2];
        [invocation setArgument:&modalResponse atIndex:3];
        [invocation setArgument:(void **)&contextInfo atIndex:4];
        [invocation invoke];
    }];
}

- (void)beginSheetModalForWindow:(NSWindow *)sheetWindow
               completionHandler:(void (^)(NSModalResponse returnCode))handler
{
    [_reserved[0] beginSheetModalForWindow:sheetWindow completionHandler:handler];
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

#pragma mark - Forwarding accessors

@dynamic title;
@dynamic message;
@dynamic attributes;
@dynamic authError;
@dynamic flags;
@dynamic targetName;
@dynamic selectedCredential;
@dynamic lastError;

- (BOOL)persist
{
    return [self->_reserved[0] persist];
}

- (void)setPersist:(BOOL)persist
{
    [self->_reserved[0] setPersist:persist];
}

static void _CUIIdentityPickerForwardPropertySetter(CUIIdentityPicker *self, SEL _cmd, id arg)
{
    [self->_reserved[0] performSelector:_cmd withObject:arg];
}

static id _CUIIdentityPickerForwardPropertyGetter(CUIIdentityPicker *self, SEL _cmd)
{
    return [self->_reserved[0] performSelector:_cmd];
}

NSString *_CUIIdentityPickerPropertyForSelector(SEL aSEL)
{
    NSString *methodName = NSStringFromSelector(aSEL);
    NSString *propertyName;

    if ([methodName hasPrefix:@"set"]) {
        NSString *firstLetter = [[methodName substringWithRange:NSMakeRange(3, 1)] lowercaseString];
        propertyName = [firstLetter stringByAppendingString:[methodName substringWithRange:NSMakeRange(4, [methodName length] - 5)]];
    } else {
        propertyName = methodName;
    }

    return propertyName;
}

BOOL _CUIIdentityPickerIsValidProperty(SEL aSEL)
{
    return !!class_getProperty([CUIIdentityPicker class], [_CUIIdentityPickerPropertyForSelector(aSEL) UTF8String]);
}

+ (BOOL)resolveInstanceMethod:(SEL)aSEL
{
    BOOL bResolves = [super resolveInstanceMethod:aSEL];

    if (!bResolves && _CUIIdentityPickerIsValidProperty(aSEL)) {
        NSAssert([CUIIdentityPickerInternal instancesRespondToSelector:aSEL],
                 ([NSString stringWithFormat:@"CUIIdentityPickerInternal must support associated class property %@", NSStringFromSelector(aSEL)]));

        if ([NSStringFromSelector(aSEL) hasPrefix:@"set"])
            class_addMethod(self, aSEL, (IMP)_CUIIdentityPickerForwardPropertySetter, "v@:@");
        else if (class_getProperty(self, sel_getName(aSEL)))
            class_addMethod(self, aSEL, (IMP)_CUIIdentityPickerForwardPropertyGetter, "@@:");
        bResolves = YES;
    }

    return bResolves;
}

@end
