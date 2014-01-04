//
//  GSSContext+CredUI.h
//  CredUI
//
//  Created by Luke Howard on 4/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

@interface GSSContext (CredUI)

- (id)initWithRequestFlags:(GSSFlags)flags
                     queue:(dispatch_queue_t)queue
            identityPicker:(CUIIdentityPicker *)identityPicker;

@end