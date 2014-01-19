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
    NSArray *objects;

    [[NSBundle credUIBundle] loadNibNamed:@"CUICredentialTile" owner:self topLevelObjects:&objects];

    [objects enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if ([obj isKindOfClass:[CUICredentialTile class]]) {
            self.view = obj;
            *stop = YES;
        }
    }];

    NSAssert(self.view != nil, @"could not load credential tile from nib");
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

    if ([keyPath isEqualTo:@"options"]) {
        CUIFieldOptions options = [[change objectForKey:NSKeyValueChangeNewKey] unsignedIntegerValue];

        [self updateView:view withFieldOptions:options];
    } else if ([keyPath isEqualTo:@"value"]) {
        CUIIdentityPickerInternal *identityPicker = (CUIIdentityPickerInternal *)[view.window delegate];
        
        [identityPicker credentialFieldDidChange:self.representedObject];
    }
}

@end
