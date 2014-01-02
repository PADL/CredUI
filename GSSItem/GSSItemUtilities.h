//
//  GSSItemUtilities.h
//  CredUI
//
//  Created by Luke Howard on 2/01/2014.
//  Copyright (c) 2014 PADL Software Pty Ltd. All rights reserved.
//

#ifndef __CredUI__GSSItemUtilities__
#define __CredUI__GSSItemUtilities__

#include <CoreFoundation/CoreFoundation.h>

class GSSItemUtilities {

public:
    
    static CFDictionaryRef createCUIAttributes(CFDictionaryRef attributes);
    static CFDictionaryRef createGSSItemAttributes(CFDictionaryRef attributes);
};

#endif /* defined(__CredUI__GSSItemUtilities__) */
