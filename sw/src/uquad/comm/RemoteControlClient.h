#ifndef UQUAD_COMM_REMOTE_CONTROL_CLIENT_H
#define UQUAD_COMM_REMOTE_CONTROL_CLIENT_H

#include "Config.h"
#include "RemoteControl.h"

namespace uquad
{
namespace comm
{
    class RemoteControlClient
        : public RemoteControl
    {
    public:
        RemoteControlClient(intrusive_ptr<base::Runloop> rl);
        ~RemoteControlClient();
        
        asio::ip::address getRemoteAddress() const;
               
    private:
        void doStart(asio::yield_context yctx, handler_t handler);
        void doStop(asio::yield_context yctx, handler_t handler);
        
        bool startDiscovery(asio::yield_context &yctx);
        void stopDiscovery(asio::yield_context &yctx);
        
        void handleDiscoveryMessage(system::error_code e, std::size_t n);
        void handleWatchdogTimer(system::error_code e);
        
        void doConnect(asio::yield_context yctx);
        bool connectChannels(asio::yield_context &yctx);
        bool connectChannel(asio::yield_context &yctx, Channel &c, int p);
        
        bool handleReceiveMessageError(system::error_code e);
        
        
    private:
        bool m_bRemoteDiscovered;
        asio::ip::udp::socket m_DiscoverySocket;
        asio::steady_timer m_WatchdogTimer;
        asio::ip::tcp::resolver m_Resolver;
        uint32_t m_DiscoveryMsgBuffer;
        asio::ip::udp::endpoint m_RemoteDiscoveryEndpoint;
        

    };
    
} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_REMOTE_CONTROL_CLIENT_H