//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTile

@synthesize credential = _credential;
@synthesize delegate = _delegate;
@synthesize selected = _selected;

- (void)dealloc
{
    NSArray *credFields = [self.credential fields];

    for (CUIField *field in credFields) {
        [field removeObserver:self.delegate forKeyPath:@"options"];
        [field removeObserver:self.delegate forKeyPath:@"value"];
    }
    
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)_updateSubviews
{
    NSArray *credFields = [self.credential fields];
    NSRect frame = self.frame;
    NSView *lastview = nil;

    frame.origin.y = frame.size.height;
    frame.size.height = 25;

    for (CUIField *field in credFields) {
        NSView *subview;
       
        if (lastview) 
            frame.origin.y -= lastview.frame.size.height;
        else
            frame.origin.y -= frame.size.height;
        
        subview = [field viewWithFrame:frame];
        if (subview == nil)
            continue;
        
        if (lastview)
            lastview.nextKeyView = subview;
        [self addSubview:subview];
        
        [field addObserver:self.delegate
                forKeyPath:@"options"
                   options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                   context:(__bridge void *)subview];
        
        [field addObserver:self.delegate
                forKeyPath:@"value"
                   options:NSKeyValueObservingOptionNew
                   context:(__bridge void *)subview];
        
        lastview = subview;
    }
}

- (void)setDelegate:(CUICredentialTileController *)delegate
{
    _delegate = delegate;
    [self _updateSubviews];
}

- (CUICredential *)credential
{
    return self.delegate.representedObject;
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (self.selected) {
        [[NSColor alternateSelectedControlColor] set];
        NSRectFill([self bounds]);
    }
}

@end
