//
//  GameNetworkBonjourManager.h
//  gl_flight
//
//  Created by Justin Brady on 12/20/17.
//

#import <Foundation/Foundation.h>
#include "gameNetwork.h"

@interface GameNetworkBonjourManager : NSObject <NSNetServiceDelegate, NSStreamDelegate, NSNetServiceBrowserDelegate>

@property (class, nonatomic, retain) GameNetworkBonjourManager* instance;
@property (nonatomic, retain) NSMutableDictionary<NSData*, NSNetService*>* browsedServices;
@property (nonatomic, retain) NSMutableArray<NSNetService*>* servicesResolving;
@property (nonatomic, retain) NSInputStream* inputStream;
@property (nonatomic, retain) NSOutputStream* outputStream;
@property (atomic, retain) NSMutableArray<NSObject*>* peers;

- (void) startServer:(const char*)name;
- (void) stopServer;
- (int) connectToServer:(const char*) name;
- (int) browse;
- (void) disconnectPeers;
- (void) sendPeer:(gameNetworkMessage*) msg peer:(int)peer_id;

@end
