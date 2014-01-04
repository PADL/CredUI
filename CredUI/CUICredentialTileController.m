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
    [self setView:[[CUICredentialTile alloc] initWithFrame:NSMakeRect(0, 0, 400, 100)]];
}

- (void)setRepresentedObject:(id)representedObject
{
    [super setRepresentedObject:representedObject];
    
    if (representedObject) {
        CUICredentialTile *tile = (CUICredentialTile *)self.view;
        tile.delegate = self;
        tile.credential = representedObject;
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
#if 0
    NSLog(@"updateView %p: %08x", view, (unsigned int)options);
#endif
    
    view.hidden = !!(options & kCUIFieldOptionsIsHidden);
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    if ([keyPath isEqualTo:@"options"]) {
        CUIFieldOptions options = [change[NSKeyValueChangeNewKey] unsignedIntegerValue];
        NSView *view = (__bridge NSView *)context;

        [self updateView:view withFieldOptions:options];
    }
}

@end
