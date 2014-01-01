//
//  CUIIdentityPickerInternal+UIFactory.m
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUIIdentityPickerInternal (UIFactory)

- (NSPanel *)_newPanel
{
    NSRect frame = NSMakeRect(0, 0, 400, 450);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask;
    NSRect rect = [NSPanel contentRectForFrameRect:frame styleMask:styleMask];
    NSPanel *panel = [[NSPanel alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];
    panel.hidesOnDeactivate = YES;
    panel.worksWhenModal = YES;
    panel.delegate = self;
    
    return panel;
}

- (NSCollectionView *)_newCollectionViewWithWindow:(NSWindow *)panel
{
    NSCollectionView *collectionView;
    
    collectionView = [[NSCollectionView alloc] initWithFrame:[[panel contentView] frame]];
    collectionView.itemPrototype = [[CUICredentialTileController alloc] init];
    collectionView.selectable = YES;
    collectionView.allowsMultipleSelection = NO;
    collectionView.autoresizingMask = (NSViewMinXMargin
                                       | NSViewWidthSizable
                                       | NSViewMaxXMargin
                                       | NSViewMinYMargin
                                       | NSViewHeightSizable
                                       | NSViewMaxYMargin);
    collectionView.autoresizesSubviews = YES;
    
    return collectionView;
}

- (NSTextField *)_newMessageTextField
{
    NSRect frame = NSMakeRect(0, 0, 400, 50);
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    textField.editable = NO;
    textField.selectable = NO;
    textField.bordered = YES;
    textField.backgroundColor = [NSColor lightGrayColor];
    
    return textField;
}

- (NSButton *)_newSubmitButton
{
    NSRect frame = NSMakeRect(0, 0, 400, 30);
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = @"OK";
    button.target = self;
    button.action = @selector(willSubmitCredential:);
    button.enabled = NO;
    
    return button;
}

@end
