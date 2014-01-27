//
//  CUIIdentityTile.m
//  CredUI
//
//  Created by Luke Howard on 29/12/2013.
//  Copyright (c) 2013 PADL Software Pty Ltd. All rights reserved.
//

@implementation CUICredentialTile

@synthesize viewPrototype = _viewPrototype;
@synthesize credential = _credential;
@synthesize delegate = _delegate;
@synthesize selected = _selected;

- (void)dealloc
{
    NSArray *credFields = [self.credential fields];

    for (CUIField *field in credFields) {
        [field removeObserver:self.delegate forKeyPath:@"options"];
    }
    
#if !__has_feature(objc_arc)
    [super dealloc];
#endif
}

- (void)_updateSubviews
{
    NSArray *credFields = [self.credential fields];
    NSRect protoFrame = self.viewPrototype.frame;
    NSView *lastview = nil;

    for (CUIField *field in credFields) {
        NSView *subview;
        NSRect frame = protoFrame;
 
        subview = [field view];
        if (subview == nil)
            continue;

        if (subview.frame.origin.x)
            frame.origin.x = subview.frame.origin.x;
        if (lastview)
            frame.origin.y = lastview.frame.origin.y - lastview.frame.size.height - 8;
        if (subview.frame.size.width || subview.frame.size.height)
            frame.size = subview.frame.size;

        subview.frame = frame;

        if (lastview)
            lastview.nextKeyView = subview;
        [self addSubview:subview];
        
        [field addObserver:self.delegate
                forKeyPath:@"options"
                   options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial
                   context:(__bridge void *)subview];
        
        lastview = subview;
    }
}

- (void)setDelegate:(CUICredentialTileController *)delegate
{
    if (delegate != _delegate) {
#if !__has_feature(objc_arc)
        [_delegate release];
        [delegate retain];
#endif
        _delegate = delegate;
    }
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
