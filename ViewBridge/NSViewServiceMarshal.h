@protocol NSViewServiceMarshal
@end

@class NSViewBridge;
@class NSViewService;

@interface NSViewServiceMarshal : NSView <NSViewServiceMarshal>
@property(nonatomic, readonly) NSViewBridge *bridge;
@property(nonatomic, retain) NSViewService *service;
@end
