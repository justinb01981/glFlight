//
//  GameNetworkBonjourManager.m
//  gl_flight
//
//  Created by Justin Brady on 12/20/17.
//

#include "GameNetworkBonjourManager.h"
#include "gameNetwork.h"
#include "gameNetworkByteOrder.h"

#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>

extern float get_time_ms_wall(void);

// private subtypes

typedef enum
{
    NOT_CONNECTED = 0,
    HALF_CONNECTED = 1,
    WAITING_EVENT = 2,
    FULLY_CONNECTED = 3
} GameNetworkPeerConnectionState;

@interface GameNetworkPeer : NSObject

@property (nonatomic, copy) NSString* name;
@property (atomic, retain) NSNetService* service;
@property (assign) GameNetworkPeerConnectionState connected;
@property (assign) int peerId;
@property (atomic, retain) NSData* address;
@property (atomic, retain) NSMutableArray<NSData*>* pendingData;
@property (atomic) int spaceIsAvailable;
@property (atomic) int threadDone;

@property (atomic, retain) NSInputStream* inputStream;
@property (atomic, retain) NSOutputStream* outputStream;

- (NSInteger) acceptConnection:(id<NSStreamDelegate>)delegate;
- (NSInteger) connectToService:(NSNetService*)host withDelegate:(id<NSStreamDelegate>)delegate;
- (instancetype) initWith:(NSNetService*) service;
- (NSInteger) sendMsg:(gameNetworkMessage*)msg;
- (void) becameConnected;
- (void) disconnect;

@end

const int PEER_ID_INITIAL = 1001;

@implementation GameNetworkPeer
{
}

@synthesize service;
@synthesize inputStream;
@synthesize outputStream;
@synthesize name;
@synthesize address;

- (instancetype)init
{
    self = [super init];
    if (self) {
        _connected = NOT_CONNECTED;
        _peerId = 0;
        service = nil;
        inputStream = nil;
        outputStream = nil;
        _pendingData = [[NSMutableArray alloc] init];
        _spaceIsAvailable = 0;
        _threadDone = 0;
    }
    return self;
}

- (instancetype) initWith:(NSNetService*) service
{
    self = [super init];
    if (self) {
        _connected = NOT_CONNECTED;
        _peerId = 0;
        service = service;
        inputStream = nil;
        outputStream = nil;
        _pendingData = [[NSMutableArray alloc] init];
        _spaceIsAvailable = 0;
        _threadDone = 0;
    }
    return self;
}

- (void)dealloc
{
    [self disconnect];
    [super dealloc];
}

- (NSInteger) acceptConnection:(id<NSStreamDelegate>)delegate
{
    if((inputStream != nil && outputStream != nil) ||
        [service getInputStream:&inputStream outputStream:&outputStream])
    {
        NSThread* t = [[NSThread alloc] initWithBlock:^{
            [inputStream setDelegate:delegate];
            [inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            
            [outputStream setDelegate:delegate];
            [outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
            
            [inputStream open];
            [outputStream open];
            
            while(!_threadDone &&
                  [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:1]])
            {
            }
            
            NSLog(@"acceptConnection runLoop exiting");
        }];
        [t start];
        
        return 1;
    }
    return 0;
}

- (NSInteger) connectToService:(NSNetService *)hostService withDelegate:(id<NSStreamDelegate>)delegate
{
    service = hostService;
    
    // TODO: must retain these in peer!
    NSInputStream* inputStreamSend;
    NSOutputStream* outputStreamSend;
    
    if(![service getInputStream:&inputStreamSend outputStream:&outputStreamSend]) return -1;
    //[NSStream getStreamsToHostWithName:[hostService hostName] port:GAME_NETWORK_PORT inputStream:&inputStreamSend outputStream:&outputStreamSend];
    
    [inputStreamSend setDelegate:GameNetworkBonjourManager.instance];
    [outputStreamSend setDelegate:GameNetworkBonjourManager.instance];
    
    [inputStreamSend scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [outputStreamSend scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    self.inputStream = inputStreamSend;
    self.outputStream = outputStreamSend;
    
    [inputStreamSend open];
    [outputStreamSend open];

    return 1;
}

- (NSInteger) sendMsg:(gameNetworkMessage*)msg
{
    if(self.connected != FULLY_CONNECTED)
    {
        NSLog(@"sendMsg but not FULLY_CONNECTED, enqueuing");
        
        [self.pendingData addObject: [NSData dataWithBytes:msg length:sizeof(*msg)]];
        return sizeof(gameNetworkMessage);
    }
    
    if(!self.spaceIsAvailable)
    {
        NSLog(@"sendMsg dropping data");
        return sizeof(*msg);
    }
    
    NSInteger w = [outputStream write:(uint8_t*)msg maxLength:sizeof(*msg)];
    if(w != sizeof(*msg))
    {
        if([[service type] containsString:@"tcp"]) [self disconnect];
        w = 0;
    }
    
    //if(w > 0) NSLog(@"%@ sendMsg wrote %u bytes", self, (unsigned int) w);
    
    return w;
}

- (void) becameConnected
{
    if(self.connected < FULLY_CONNECTED) self.connected = FULLY_CONNECTED;
    
    if([self.pendingData count] > 0)
    {
        NSLog(@"BONJOUR becameConnected\n");
        
        gameNetworkMessage* msg = (gameNetworkMessage*) [[self.pendingData firstObject] bytes];
        
        [self sendMsg:msg];
        
        [self.pendingData removeObjectAtIndex: 0];
    }
}

- (void) disconnect
{
    _threadDone = 1;
    
    if(self.connected == NOT_CONNECTED) return;
    self.connected = NOT_CONNECTED;
    
    if(inputStream)
    {
        //[inputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [inputStream close];
        inputStream = nil;
    }
    
    if(outputStream)
    {
        //[outputStream removeFromRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        [outputStream close];
        outputStream = nil;
    }
}

@end

// implementation
@implementation GameNetworkBonjourManager
{
    NSNetService* server;    NSHost* serverHost;
    NSMutableArray<NSString*>* foundDomains;
    NSMutableArray<NSNetServiceBrowser*>* browsers;
    NSString* type;
    int peerIdNext;
}

@synthesize browsedServices;
@synthesize servicesResolving;
@synthesize inputStream;
@synthesize outputStream;
@synthesize peers;

//@dynamic instance;

// singleton
static GameNetworkBonjourManager* instance;

+ (instancetype)instance
{
    if(!instance)
    {
        instance = [[GameNetworkBonjourManager alloc] init];
    }
    
    return instance;
}

+ (void)setInstance:(GameNetworkBonjourManager*)val
{
    instance = val;
}

- (BOOL) isConnected {
    __block BOOL found = NO;
    NSMutableArray<GameNetworkPeer*>* peers = (void*) self.peers;
    
    [peers enumerateObjectsUsingBlock:^(GameNetworkPeer * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if(obj.connected == FULLY_CONNECTED) {
            found = YES;
            *stop = YES;
        }
    }];
    return found;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        browsers = [[NSMutableArray alloc] init];
        
        foundDomains = [[NSMutableArray alloc] init];
        
        browsedServices = [[NSMutableDictionary alloc] init];
        
        servicesResolving = [[NSMutableArray alloc] init];
        
        peers = [[NSMutableArray alloc] init];
        
        peerIdNext = PEER_ID_INITIAL;
        
        type = @"_d0gf1ght._tcp.";
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void) startServer:(const char*) name
{
    if(server == nil)
    {
        server = [[NSNetService alloc] initWithDomain:@"" type:type name:[NSString stringWithUTF8String:name] port:GAME_NETWORK_PORT_BONJOUR];
        server.includesPeerToPeer = YES;
        [server setDelegate:self];
        [server publishWithOptions: NSNetServiceListenForConnections | NSNetServiceNoAutoRename];
    }
}

- (void) stopServer
{
    if(server != nil)
    {
        [server stop];
        server = nil;
    }
}

- (int) connectToServer:(NSNetService*) serviceFound
{
    __block int peerIdAdded = -1;

    /*
    [[self.browsedServices allKeys] enumerateObjectsUsingBlock:^(NSData * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        NSNetService* service = browsedServices[obj];
     
        if([service.name isEqualToString:[NSString stringWithUTF8String:name]] &&
           service.includesPeerToPeer)
        {
            serviceFound = service;
     
            *stop = YES;
        }
    }];
    */
    
    if(serviceFound && serviceFound.addresses.count > 0)
    {
        GameNetworkPeer* p = [[GameNetworkPeer alloc] initWith:serviceFound];
        
        [peers addObject:p];
        
        p.peerId = PEER_ID_INITIAL;
        peerIdAdded = p.peerId;
        
        [serviceFound setDelegate:self];
        
        NSThread* thread = [[NSThread alloc] initWithBlock:^{
            [p connectToService:serviceFound withDelegate:self];
            
            while(!p.threadDone &&
                  [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:1]])
            {
            }
            
            NSLog(@"connectToServer runLoop exiting");
        }];
        [thread start];
    }
    
    return peerIdAdded;
}

- (int) browse
{
    [peers removeAllObjects];
    
    [browsers enumerateObjectsUsingBlock:^(NSNetServiceBrowser * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [obj stop];
    }];
    
    [browsers removeAllObjects];
    
    [browsers addObject:[[NSNetServiceBrowser alloc] init]];
    
    browsers[0].includesPeerToPeer = YES;
    [browsers[0] setDelegate:self];
    
    [foundDomains removeAllObjects];
    [browsers[0] searchForBrowsableDomains];
    
    [browsedServices removeAllObjects];
    
    [servicesResolving removeAllObjects];
    
    return 1;
}

- (void) browseEnd
{
    [browsers enumerateObjectsUsingBlock:^(NSNetServiceBrowser * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [obj stop];
    }];
    [browsers removeAllObjects];
}

- (void) sendPeer:(gameNetworkMessage*) msg peer:(int)peer_id
{
    NSMutableArray<GameNetworkPeer*>* peers = (void*) self.peers;
    __block gameNetworkMessage* msg_ = msg;
    [peers enumerateObjectsUsingBlock:^(GameNetworkPeer * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        if(obj.peerId == peer_id)
        {
            //[obj.pendingData addObject:[NSData dataWithBytes:msg length:sizeof(gameNetworkMessage)]];
            [obj sendMsg:(gameNetworkMessage*) msg];
            *stop = YES;
        }
    }];
}

- (void) addPeer:(gameNetworkPlayerInfo*)playerInfo
{
    GameNetworkPeer* peer = [[GameNetworkPeer alloc] init];
    
    peer.peerId = playerInfo->player_id;
    peer.address = [NSMutableData dataWithCapacity:28];
    *((int*)[peer.address bytes]) = peer.peerId;
    
    [peers addObject:peer];
}

- (void) disconnectPeers
{
    NSMutableArray<GameNetworkPeer*>* peers = (void*) self.peers;
    [peers enumerateObjectsUsingBlock:^(GameNetworkPeer * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [obj disconnect];
    }];
    
    [peers removeAllObjects];
}

// NSNetServiceDelegate delegate

- (void)netServiceDidPublish:(NSNetService *)sender
{
    NSLog(@"netServiceDidPublish in domain %@", [sender domain]);
 
    [sender scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [sender startMonitoring];
    
    [sender getInputStream:&inputStream outputStream:&outputStream];
    
    [inputStream setDelegate:self];
    [inputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [outputStream setDelegate:self];
    [outputStream scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    
    [inputStream open];
    [outputStream open];
}

- (void)netService:(NSNetService *)sender didNotPublish:(NSDictionary<NSString *,NSNumber *> *)errorDict
{
    NSLog(@"netServiceDidNotPublish");
    console_write("ERROR: netService didNotPublish");
}

- (void)netService:(NSNetService *)sender didAcceptConnectionWithInputStream:(NSInputStream *)inputStream outputStream:(NSOutputStream *)outputStream
{
    GameNetworkPeer* peer = [[GameNetworkPeer alloc] initWith:sender];
    
    peer.inputStream = inputStream;
    peer.outputStream = outputStream;
    peer.peerId = peerIdNext++;
    
    peer.address = [NSMutableData dataWithCapacity:sizeof(struct bonjour_addr_stuffed_in_sockaddr_in)];
    ((struct bonjour_addr_stuffed_in_sockaddr_in *)[peer.address bytes])->len = sizeof(struct bonjour_addr_stuffed_in_sockaddr_in);
    ((struct bonjour_addr_stuffed_in_sockaddr_in *)[peer.address bytes])->sa_family = GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK;
    ((struct bonjour_addr_stuffed_in_sockaddr_in *)[peer.address bytes])->peer_id = peer.peerId;
    
    [sender setDelegate:self];

    [peers addObject:peer];
    
    [peer acceptConnection:self];
}

- (void)stream:(NSStream *)stream handleEvent:(NSStreamEvent)eventCode
{
    __block GameNetworkPeer* peer = nil;
    __block int peerId;
    NSMutableArray<GameNetworkPeer*>* peers = (void*) self.peers;
    
    //NSLog(@"NSStream %@ event %u", [stream debugDescription], (unsigned int) eventCode);

    // find peer
    [peers enumerateObjectsUsingBlock:^(GameNetworkPeer * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
    {
        if(stream == obj.inputStream || stream == obj.outputStream || obj.connected == NOT_CONNECTED || obj.connected == HALF_CONNECTED)
        {
            peer = obj;
            peerId = peer.peerId;
            *stop = YES;
        }
    }];
    
    if(!peer)
    {
        return;
    }
        
    if([stream isKindOfClass:NSOutputStream.class])
    {
        peer.outputStream = (NSOutputStream*) stream;
    }
    else if([stream isKindOfClass:NSInputStream.class])
    {
        peer.inputStream = (NSInputStream*) stream;
    }
    
    switch(eventCode) {
            
        case NSStreamEventOpenCompleted: {
            peer.connected = peer.inputStream && peer.outputStream ? WAITING_EVENT : HALF_CONNECTED;
            if(peer.connected == WAITING_EVENT)
            {
                NSLog(@"BONJOUR connected\n");
            }
        } break;
            
        case NSStreamEventHasSpaceAvailable: {
            //NSLog(@"BONJOUR NSStreamEventHasSpaceAvailable\n");
            peer.spaceIsAvailable = true;
            if([peer outputStream] == stream)
            {
                [peer becameConnected];
            }
        } break;
            
        case NSStreamEventHasBytesAvailable: {
            //NSLog(@"BONJOUR NSStreamEventHasBytesAvailable\n");
            NSInteger   bytesRead;
            gameNetworkMessageQueued* msg;
            
            msg = (gameNetworkMessageQueued*) malloc(sizeof(gameNetworkMessageQueued));
            
            memset(msg, 0, sizeof(*msg));
            bytesRead = [peer.inputStream read:(uint8_t*)&msg->msg maxLength:sizeof(gameNetworkMessage)];
            
            if(bytesRead == sizeof(gameNetworkMessage))
            {
                gameMessage_from_nbo(&msg->msg);
            }
            
            struct bonjour_addr_stuffed_in_sockaddr_in *p_saddr = (struct bonjour_addr_stuffed_in_sockaddr_in *) msg->srcAddr.storage;
            p_saddr->len = msg->srcAddr.len = sizeof(struct bonjour_addr_stuffed_in_sockaddr_in);
            p_saddr->sa_family = GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK;
            p_saddr->peer_id = peerId;
            
            msg->receive_time = get_time_ms_wall();
            
            if (bytesRead <= 0) {
                // Do nothing; we'll handle EOF and error in the
                // NSStreamEventEndEncountered and NSStreamEventErrorOccurred case,
                // respectively.
                free(msg);
            }
            else
            {
                //NSLog(@"NSStreamEventHasBytesAvailable: read %u bytes", (unsigned int) bytesRead);
                
                if(gameNetworkState.gameNetworkHookOnMessage && gameNetworkState.gameNetworkHookOnMessage(&msg->msg, &msg->srcAddr))
                {
                    free(msg);
                }
                else
                {
                    game_lock_lock(&gameNetworkState.msgQueue.lock);
                    
                    gameNetworkMessageQueued* cur = &gameNetworkState.msgQueue.head;
                    while(cur->next != NULL)
                    {
                        cur = cur->next;
                    }
                    cur->next = msg;
                    
                    game_lock_unlock(&gameNetworkState.msgQueue.lock);
                    
                    extern void do_game_network_read_bonjour(void);
                    do_game_network_read_bonjour();
                }
            }
        } break;
            
        default:
            assert(NO);
            // fall through
        case NSStreamEventErrorOccurred: {
            NSLog(@"NSStreamEventErrorOccurred: %@/%@",  [[stream streamError] localizedDescription], [[stream streamError] localizedFailureReason]);
            gameNetwork_removePlayerBonjour(peer.peerId);
            // the above call will disconnect/remove
            //[peer disconnect];
            //[peers removeObject: peer];
        } break;
            
        case NSStreamEventEndEncountered: {
            NSLog(@"NSStreamEventEndEncountered for peer: %@",  [peer debugDescription]);
            gameNetwork_removePlayerBonjour(peer.peerId);
            // the above call will disconnect/remove
            //[peer disconnect];
            //[peers removeObject: peer];

        } break;
    }
}

// NSNetServiceBrowserDelegate

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindDomain:(NSString *)domainString moreComing:(BOOL)moreComing
{
    [foundDomains addObject:domainString];
    
    NSNetServiceBrowser* newBrowser = [[NSNetServiceBrowser alloc] init];
    [browsers addObject:newBrowser];
    
    [foundDomains enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        [newBrowser setDelegate:self];
        newBrowser.includesPeerToPeer = TRUE;
        [newBrowser searchForServicesOfType:type inDomain:obj];
    }];
}

- (void) netServiceDidResolveAddress:(NSNetService *)sender
{
    NSLog(@"netServiceBrowser did resolve service address:%@", [sender.addresses.lastObject debugDescription]);
    
    [sender.addresses enumerateObjectsUsingBlock:^(NSData * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        self.browsedServices[obj] = sender;
    }];
    
    [servicesResolving removeObject:sender];
    
    if(gameNetworkState.gameNetworkHookGameDiscovered != NULL) gameNetworkState.gameNetworkHookGameDiscovered(sender.hostName.UTF8String);
}

- (void) netService:(NSNetService *)sender didNotResolve:(NSDictionary<NSString *,NSNumber *> *)errorDict
{
    NSLog(@"netServiceBrowser did not resolve: %@", [errorDict description]);
    [servicesResolving removeObject:sender];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didFindService:(NSNetService *)service moreComing:(BOOL)moreComing
{
    [self.browsedServices enumerateKeysAndObjectsUsingBlock:^(NSData * _Nonnull key, NSNetService * _Nonnull obj, BOOL * _Nonnull stop) {
        if([obj.name isEqualToString:server.name])
        {
            [self.browsedServices removeObjectForKey:key];
            *stop = YES;
        }
    }];
    
    [service setDelegate:self];
    
    [servicesResolving addObject:service];
    
    [service scheduleInRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    [service resolveWithTimeout:2.0];
}

- (void)netServiceBrowser:(NSNetServiceBrowser *)browser didNotSearch:(NSDictionary *)errorDict
{
    NSLog(@"netServiceBrowser did not search: %@", [errorDict description]);
    [browser stop];
    
    [browsers removeObject:browser];
}

@end

// C bridging

int GameNetworkBonjourManagerHost(const char* name, int* sock_out)
{
    gameNetworkState.hostInfo.bonjour_lan = 1;
    [GameNetworkBonjourManager.instance startServer: name];
    
    return 1;
}

void GameNetworkBonjourManagerSendMessageToPeer(uint8_t* msg_, int peer_id)
{
    gameNetworkMessage* msg = (gameNetworkMessage*) msg_;
    
    [GameNetworkBonjourManager.instance sendPeer:msg peer:peer_id];
}

int GameNetworkBonjourManagerBrowseBegin()
{
    //GameNetworkBonjourManager.instance = nil;
    [GameNetworkBonjourManager.instance browse];
    return 1;
}

int GameNetworkBonjourManagerBrowseEnd(gameNetworkAddress* server_address_ptr)
{
    GameNetworkBonjourManager* m = GameNetworkBonjourManager.instance;
    [m browseEnd];
    
    if(m.browsedServices.count == 0) return 0;
    
    [m connectToServer: m.browsedServices[m.browsedServices.allKeys.firstObject]];
    
    memset(server_address_ptr->storage, 0, sizeof(server_address_ptr->storage));
    
    // -- at this point we have already either connected or failed
    ((struct bonjour_addr_stuffed_in_sockaddr_in *) server_address_ptr->storage)->len = sizeof(struct bonjour_addr_stuffed_in_sockaddr_in);
    ((struct bonjour_addr_stuffed_in_sockaddr_in *) server_address_ptr->storage)->sa_family = GAME_NETWORK_BONJOUR_ADDRFAMILY_HACK;
    ((struct bonjour_addr_stuffed_in_sockaddr_in *) server_address_ptr->storage)->peer_id = /*GAME_NETWORK_PLAYER_ID_HOST*/ PEER_ID_INITIAL;
    server_address_ptr->len = sizeof(struct bonjour_addr_stuffed_in_sockaddr_in);
    
    return 1;
}

void GameNetworkBonjourManagerDisconnect()
{
    [GameNetworkBonjourManager.instance stopServer];
    [GameNetworkBonjourManager.instance disconnectPeers];
    [GameNetworkBonjourManager.instance browseEnd];
}

int GameNetworkBonjourManagerDisconnectPeer(int peer_id)
{
    __block GameNetworkPeer* p = nil;
    [[[GameNetworkBonjourManager instance] peers] enumerateObjectsUsingBlock:^(NSObject * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        p = (GameNetworkPeer*) obj;
        if(p.peerId == peer_id)
        {
            [p disconnect];
        }
    }];
    
    if(p) [[[GameNetworkBonjourManager instance] peers] removeObject:p];
    
    return 1;
}
