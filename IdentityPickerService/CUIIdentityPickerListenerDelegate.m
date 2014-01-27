//
//  CUIIdentityPickerListenerDelegate.m
//  CredUI
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/CUIVBIdentityPicker.h>

#import "CUIIdentityPickerListenerDelegate.h"
#import "CUIIdentityPickerService.h"

@implementation CUIIdentityPickerListenerDelegate

@synthesize listener = _listener;

- (void)didStartAsServiceNamed:(NSString *)serviceName
{
}

- (BOOL)listener:(NSXPCListener *)listener shouldAcceptNewConnection:(NSXPCConnection *)connection
{
    CUIIdentityPickerService *identityPickerService = [[CUIIdentityPickerService alloc] init];
    
    connection.exportedInterface = [NSXPCInterface interfaceWithProtocol:@protocol(CUIIdentityPickerProxyProtocol)];
    connection.exportedObject = identityPickerService;
    connection.remoteObjectInterface = connection.exportedInterface;
 
#if !__has_feature(objc_arc)
    [identityPickerService release];
#endif

    [connection resume];
    
    return YES;
}

@end
