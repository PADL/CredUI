//
//  CUIIdentityPicker.m
//  CredUI
//
//  Created by Luke Howard on 7/11/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

#import "CUIIdentityPicker.h"

@implementation CUIIdentityPicker
- init
{
    if ((self = [super init]) == nil)
        return nil;
    
    return self;
}

- (NSInteger)runModal
{
    return NSCancelButton;
}

- (void)runModalForWindow:(NSWindow *)window
            modalDelegate:(id)delegate
           didEndSelector:(SEL)didEndSelector
              contextInfo:(void *)contextInfo
{
}
@end