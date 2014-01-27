@protocol NSRemoteViewMarshal
- (void)setRemoteObject:(id)object forKey:(NSString *)key withReply:(id)block;
@end

@interface NSRemoteViewMarshal : NSObject <NSRemoteViewMarshal>
@property(readonly) NSViewBridge *bridge; // @synthesize bridge=_bridge;
@property(readonly) NSRemoteView *view; // @synthesize view=_view;
@property(readonly) BOOL isInvalid;
@end
