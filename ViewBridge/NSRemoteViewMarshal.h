@protocol NSRemoteViewMarshal
- (void)setRemoteObject:(NSObject<NSSecureCoding> *)object forKey:(NSString *)key withReply:(void (^)(NSError *error))block;
- (void)registerBridgeKey:(NSString *)key defaultObject:(id)defaultObject owner:(NSViewBridgeKeyOwner)owner withReply:(void (^)(NSError *error))block;
@end

@interface NSRemoteViewMarshal : NSObject <NSRemoteViewMarshal>
@property(readonly) NSViewBridge *bridge;
@property(readonly) NSRemoteView *view;
@property(readonly) BOOL isInvalid;
@end
