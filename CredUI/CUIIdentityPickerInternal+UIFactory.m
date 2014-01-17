//
//  CUIIdentityPickerInternal+UIFactory.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIIdentityPickerInternal (UIFactory)

- (BOOL)_loadViews
{
    NSBundle *frameworkBundle = [NSBundle bundleForClass:self.class];
    BOOL bLoaded;
    NSArray *objects = nil;

    bLoaded = [frameworkBundle loadNibNamed:@"CUIIdentityPicker" owner:self topLevelObjects:&objects];
    NSAssert(bLoaded, @"Could not load identity picker nib");
    
    if (!bLoaded)
        return NO;

    if ((self.flags & CUIFlagsShowSaveCheckBox) == 0)
        [self.persistCheckBox setHidden:YES];

    CUICredUIContext uic = { .version = 0, .parentWindow = (__bridge CFTypeRef)self.identityPickerPanel };
    [self setCredUIContext:&uic properties:kCUICredUIContextPropertyParentWindow];

    return YES;
}

@end
