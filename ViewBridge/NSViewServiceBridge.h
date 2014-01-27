#import <ViewBridge/NSViewBridge.h>

@class NSViewServiceMarshal;

@interface NSViewServiceBridge : NSViewBridge
{
    NSViewServiceMarshal *_marshal;
}

@property(readonly) NSViewServiceMarshal *viewServiceMarshal; // @synthesize viewServiceMarshal=_marshal;
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context;
- (int)processIdentifier;
- (audit_token_t)auditToken;
- (id)initWithServiceMarshal:(NSViewServiceMarshal *)serviceMarshal;

@end

extern int
NSViewServiceApplicationMain(int argc, const char *argv[]);

