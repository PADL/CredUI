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
    NSRect frame = NSMakeRect(0, 0, 360, 480);
    NSUInteger styleMask = NSTitledWindowMask | NSClosableWindowMask;
    NSRect rect = [NSPanel contentRectForFrameRect:frame styleMask:styleMask];
    NSPanel *panel = [[NSPanel alloc] initWithContentRect:rect styleMask:styleMask backing:NSBackingStoreBuffered defer:YES];
    panel.hidesOnDeactivate = YES;
    panel.worksWhenModal = YES;
    panel.delegate = self;
    
    return panel;
}

- (NSCollectionView *)_newCollectionViewEnclosedInView:(NSView *)view
{
    NSCollectionView *collectionView;
    
    collectionView = [[NSCollectionView alloc] initWithFrame:[view frame]];
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
    NSRect frame = NSMakeRect(0, 0, 360, 50);
    NSTextField *textField = [[NSTextField alloc] initWithFrame:frame];
    
    textField.editable = NO;
    textField.selectable = NO;
    textField.bordered = YES;
    textField.backgroundColor = [NSColor lightGrayColor];
    
    return textField;
}

- (NSButton *)_newSubmitButton
{
    NSRect frame = NSMakeRect(0, 0, 360, 30);
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = @"OK";
    button.target = self;
    button.action = @selector(willSubmitCredential:);
    button.enabled = NO;
    
    return button;
}

- (NSButton *)_newPersistCheckBox
{
    NSRect frame = NSMakeRect(0, 25, 360, 30);
    NSButton *button = [[NSButton alloc] initWithFrame:frame];
    
    button.title = @"Save credential";
    button.target = self;
    button.buttonType = NSSwitchButton;
    button.action = @selector(didClickPersist:);
    button.enabled = YES;
    button.state = self.persist;

    return button;
}

@end
