#include "game/gameNetwork.h"

#ifdef __cplusplus
extern "C" {
#endif

    int GameNetworkBonjourManagerHost(const char* name, int* sock_out)
    {
        return 0;
    }
    
    void GameNetworkBonjourManagerSendMessageToPeer(uint8_t* msg_, int peer_id)
    {
        return;
    }
    
    int GameNetworkBonjourManagerBrowseBegin()
    {
        return 0;
    }
    
    int GameNetworkBonjourManagerBrowseEnd(gameNetworkAddress* server_address_ptr)
    {
        return 0;
    }
    
    void GameNetworkBonjourManagerDisconnect()
    {
    }
    
    int GameNetworkBonjourManagerDisconnectPeer(int peer_id)
    {
        return 0;
    }
    
    void
    AppDelegateOpenURL(const char* url)
    {
        return;
    }
    
    int
    AppDelegateIsMultiplayerEager()
    {
        return 0;
    }

#ifdef __cplusplus
}
#endif

