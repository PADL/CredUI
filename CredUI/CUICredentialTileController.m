//
//  CUICredentialTileController.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTileController

- (void)loadView
{
    CUICredentialTile *tile = [[CUICredentialTile alloc] initWithFrame:NSMakeRect(0, 0, 400, 100)];
    
    [self setView:tile];
    
#if !__has_feature(objc_arc)
    [tile release];
#endif
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];
    
    if (representedObject) {
        CUICredentialTile *tile = (CUICredentialTile *)self.view;
        tile.delegate = self;
    }
}

- (void)setSelected:(BOOL)flag
{
    [super setSelected:flag];
    [(CUICredentialTile *)self.view setSelected:flag];
    [self.view setNeedsDisplay:YES];
}

- (void)updateView:(NSView *)view withFieldOptions:(CUIFieldOptions)options
{
    view.hidden = !!(options & kCUIFieldOptionsIsHidden);
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    NSView *view = (__bridge NSView *)context;

#if 0
    NSLog(@"observeValueForKeyPath:%@ ofObject:%@ change:%@ context:%@", keyPath, object, change[NSKeyValueChangeNewKey], (__bridge id)context);
#endif

    if ([keyPath isEqualTo:@"options"]) {
        CUIFieldOptions options = [[change objectForKey:NSKeyValueChangeNewKey] unsignedIntegerValue];

        [self updateView:view withFieldOptions:options];
    } else if ([keyPath isEqualTo:@"value"]) {
        CUIIdentityPickerInternal *identityPicker = (CUIIdentityPickerInternal *)[view.window delegate];
        
        [identityPicker credentialFieldDidChange:self.representedObject];
    }
}

@end
