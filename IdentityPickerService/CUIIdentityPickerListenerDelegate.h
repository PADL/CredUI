//
//  CUIIdentityPickerListenerDelegate.h
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <Foundation/NSXPCConnection.h>

@interface CUIIdentityPickerListenerDelegate : NSObject <NSXPCListenerDelegate>
{
    NSXPCListener *_listener;
}

@property(nonatomic, retain) NSXPCListener *listener;

@end
