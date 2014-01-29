typedef NS_ENUM(unsigned char, NSViewBridgePhase) {
    NSViewBridgePhaseInvalid        = 0,
    NSViewBridgePhaseInit           = 1,
    NSViewBridgePhaseConfig         = 2,
    NSViewBridgePhaseRun            = 3
};

typedef NS_ENUM(unsigned char, NSViewBridgeKeyOwner) {
    NSViewBridgeKeyOwnerRemote = 0,
    NSViewBridgeKeyOwnerService = 1,
    NSViewBridgeKeyOwnerPhased = 2
};

@protocol NSViewBridgeKVOBuddy
@end

@interface NSViewBridge : NSObject
{
    NSMutableDictionary *_dict;
    NSObject<NSViewBridgeKVOBuddy> *_kvoBuddy;
    NSMutableSet *_nonLocalChangesInProgress;
}

@property(nonatomic, retain) id <NSViewBridgeKVOBuddy> kvoBuddy;
@property(nonatomic, readonly) audit_token_t auditToken;
@property(nonatomic, readonly) int processIdentifier;

- (void)setObject:(id)object forKey:(NSString *)key withKVO:(BOOL)kvo;
- (void)setObject:(id)object forKey:(NSString *)key;
- (id)valueForKey:(NSString *)key;
- (NSArray *)allKeys;

- (void)registerKey:(NSString *)key defaultObject:(id)object owner:(NSViewBridgeKeyOwner)owner;
- (BOOL)keyIsRelevantToBuddy:(NSString *)key;

- (NSViewBridgeKeyOwner)ownerForKey:(NSString *)key;
- (id)objectForKey:(NSString *)key;
- (BOOL)hasObject:(id)anObject forKey:(NSString *)key;
- (BOOL)hasObjectForKey:(NSString *)key;
- (BOOL)hasKey:(NSString *)key;

@end
