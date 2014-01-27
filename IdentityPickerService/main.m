//
//  main.m
//  IdentityPickerService
//
//  Created by Luke Howard on 27/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <ViewBridge/ViewBridge.h>

#import "CUIIdentityPickerService.h"
#import "CUIIdentityPickerListenerDelegate.h"

int main(int argc, const char *argv[])
{
    NSXPCListener *anonymousListener = [NSXPCListener anonymousListener];
    NSXPCSharedListener *sharedListener = [NSXPCSharedListener sharedServiceListener];
    CUIIdentityPickerListenerDelegate *delegate = [[CUIIdentityPickerListenerDelegate alloc] init];
    
    delegate.listener = anonymousListener;
    [anonymousListener setDelegate:delegate];
    [sharedListener addListener:anonymousListener withName:@"IdentityPickerService"];
    
#if !__has_feature(objc_arc)
    [delegate release];
#endif
    
    return NSViewServiceApplicationMain(argc, argv);
}