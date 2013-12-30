//
//  CUIField.h
//  CredUI
//
//  Created by Luke Howard on 30/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@interface CUIField : NSObject <NSTextFieldDelegate>

@property(nonatomic, readonly, assign) CUIFieldClass fieldClass;
@property(nonatomic, readonly, assign) NSString *title;
@property(nonatomic, readonly, assign) NSString *defaultValue;

- initWithClass:(CUIFieldClass)fieldClass
          title:(NSString *)title
   defaultValue:(id)defaultValue
       delegate:(void(^)(CUIFieldRef, CFTypeRef))fieldDidChange;

- (CUIFieldRef)_fieldRef;
- (void)setValue:(id)aValue;

- (void)didSubmit:(id)sender;

@end
