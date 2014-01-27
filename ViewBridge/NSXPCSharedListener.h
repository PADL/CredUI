@interface NSXPCSharedListener : NSObject
{
    NSMutableDictionary *_listeners;
    NSHashTable *_delegates;
}

+ (void)warmUpClassNamed:(NSString *)name inServiceNamed:(NSString *)service;
+ (instancetype)sharedServiceListener;
+ (instancetype)connectionForListenerNamed:(NSString *)listenerName fromServiceNamed:(NSString *)serviceName;
+ (instancetype)connectionForListenerNamed:(NSString *)listenerName fromConnection:(xpc_connection_t)conn;
+ (instancetype)connectionForListenerWithXPCName:(void *)xpcName fromConnection:(xpc_connection_t)conn;
+ (instancetype)connectionForListenerWithMessage:(void *)message fromConnection:(xpc_connection_t)conn withListenerName:(void *)listenerName;
+ (instancetype)subserviceConnectionForReply:(void *)reply withListenerName:(void *)listenerName;

@property(nonatomic, retain) NSMutableDictionary *listeners;

- (void)addListener:(NSXPCListener *)listener withName:(NSString *)name;
- (void)addDelegate:(NSObject *)delegate;

@end
