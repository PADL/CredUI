//
//  CUIIdentityPickerInternal+UIFactory.h
//  CredUI
//
//  Created by Luke Howard on 1/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIIdentityPickerInternal (UIFactory)

- (NSPanel *)_newPanel;
- (NSCollectionView *)_newCollectionViewEnclosedInView:(NSView *)panel;
- (NSTextField *)_newMessageTextField;
- (NSButton *)_newSubmitButton;
- (NSButton *)_newPersistCheckBox;
- _initUI;

@end
