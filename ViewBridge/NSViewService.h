@class NSViewServiceMarshal;
@class NSViewServiceBridge;

@interface NSViewService : NSViewController
{
    NSViewServiceMarshal *_marshal;
    unsigned int _invalid:1;
    unsigned int _trustRemoteKeyEvents:1;
}

@property(nonatomic, retain) NSViewServiceMarshal *marshal;
@property(nonatomic, readonly) BOOL invalid;
@property(nonatomic, readonly) NSViewServiceBridge *bridge;

+ (id)serviceBundle;
+ (void)deferBlockOntoMainThread:block;

- (void)setView:(NSView *)view;

- (NSBundle *)nibBundle;
- (NSString *)nibName;

- (NSUInteger)awakeFromRemoteView;
- (instancetype)initWithNibName:(NSString *)nibName bundle:(NSBundle *)bundle;

@end
