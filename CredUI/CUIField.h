//
//  CUIField.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

__attribute__((visibility("default")))
@interface CUIField : NSObject <NSTextFieldDelegate>

@property(nonatomic, readonly, assign) CUIFieldClass fieldClass;
@property(nonatomic, readonly, copy) NSString *title;
@property(nonatomic, readonly, copy) NSString *defaultValue;
@property(nonatomic, assign) CUIFieldOptions options; //primitive

- (CUIFieldRef)_fieldRef;
- (void)setValue:(id)aValue; //primitive
- (void)didSubmit:(id)sender;

@end
