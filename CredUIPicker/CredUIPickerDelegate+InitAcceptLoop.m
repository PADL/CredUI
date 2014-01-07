//
//  CredUIPickerDelegate+InitAcceptLoop.m
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#import <CredUI/CredUI.h>
#import <CredUI/GSSPromptForCredentials.h>
#import <CredUICore/CUIAttributes.h>

#import <GSSKit/GSSKit.h>
#import <GSSKitUI/GSSKitUI.h>

#import "CredUIPickerDelegate.h"

/*
 * Establish a GSS context: for testing, this implements both the client and server side.
 */
@implementation CredUIPickerDelegate (InitAcceptLoop)

- (void)doInitAcceptGSSContextWithIdentityPicker:(CUIIdentityPicker *)identityPicker
{
    GSSContext *initiatorCtx;
    dispatch_queue_t queue = dispatch_queue_create("com.padl.CredUIPicker.queue", DISPATCH_QUEUE_SERIAL);

    initiatorCtx = [[GSSContext alloc] initWithRequestFlags:0 //GSS_C_MUTUAL_FLAG
                                                      queue:queue
                                             identityPicker:identityPicker];
    
    if (initiatorCtx.credential == nil) {
        NSLog(@"Warning: no credential acquired for picker %@", identityPicker);
    }
    
    initiatorCtx.promptForCredentials = NO;
    initiatorCtx.window = self.window;

    [self doInitAcceptGSSContext:initiatorCtx];
}

- (void)doInitAcceptGSSContext:(GSSContext *)initiatorCtx
{
    __block GSSContext *acceptorCtx = nil;
    __block NSError *err = nil;
    __block NSData *initiatorToken = nil, *acceptorToken = nil;
    
    acceptorCtx = [[GSSContext alloc] initWithRequestFlags:0
                                                     queue:initiatorCtx.queue
                                               isInitiator:NO];
    
    acceptorCtx.credential = [GSSCredential credentialWithName:initiatorCtx.targetName
                                                     mechanism:initiatorCtx.mechanism
                                                    attributes:@{GSSCredentialUsage : GSSCredentialUsageAccept}
                                                         error:&err];
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
        do {
            dispatch_semaphore_t sema = dispatch_semaphore_create(0);
            
            [initiatorCtx stepWithData:acceptorToken
                     completionHandler:^(NSData *outputToken, NSError *error) {
                         initiatorToken = outputToken;
                         dispatch_semaphore_signal(sema);
                     }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            
            if ([initiatorCtx didError] || ![initiatorToken length])
                break;
            
            NSLog(@"Sending initiator token %@", initiatorToken);
            
            [acceptorCtx stepWithData:initiatorToken
                    completionHandler:^(NSData *outputToken, NSError *error) {
                        acceptorToken = outputToken;
                        dispatch_semaphore_signal(sema);
                    }];
            dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
            if ([acceptorCtx didError])
                break;
            
            NSLog(@"Sending acceptor token %@", acceptorToken);
        } while ([initiatorCtx isContinueNeeded]);
        
        NSLog(@"Initiator status %ld acceptor status %ld", [[initiatorCtx lastError] code], [[acceptorCtx lastError] code]);
    });
}

@end
