#ifdef __cplusplus
extern "C" {
#endif

#include "gameNetwork.h"



    int GameNetworkBonjourManagerHost(const char* name, int* sock_out)
    {
        return 0;
    }
    
    void GameNetworkBonjourManagerSendMessageToPeer(uint8_t* msg_, int peer_id)
    {
        return;
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
