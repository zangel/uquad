#ifndef UQUAD_COMM_REMOTE_CONTROL_SERVER_H
#define UQUAD_COMM_REMOTE_CONTROL_SERVER_H

#include "Config.h"
#include "RemoteControl.h"

namespace uquad
{
namespace comm
{
    class RemoteControlServer
        : public RemoteControl
    {
    public:
        RemoteControlServer(intrusive_ptr<base::Runloop> rl);
        ~RemoteControlServer();
        
        asio::ip::address getRemoteAddress() const;

    private:
        void doStart(asio::yield_context yctx, handler_t handler);
        void doStop(asio::yield_context yctx, handler_t handler);
        
        bool startDiscovery(asio::yield_context &yctx);
        void stopDiscovery(asio::yield_context &yctx);
        
        void handleDiscoveryTimer(system::error_code e);
        void handleBroadcastDiscovery(system::error_code e, std::size_t n);
        
        bool startListening(asio::yield_context &yctx);
        bool startListening(asio::yield_context &yctx, asio::ip::tcp::acceptor &a, int p);
        
        void stopListening(asio::yield_context &yctx);
        void stopListening(asio::yield_context &yctx, asio::ip::tcp::acceptor &a);
        
        
        void doAccept(asio::yield_context yctx);
        bool acceptChannels(asio::yield_context &yctx);
        bool acceptChannel(asio::yield_context &yctx, Channel &c, asio::ip::tcp::acceptor &a, asio::ip::tcp::socket &s);
        
        bool handleReceiveMessageError(system::error_code e);
        
    private:
        asio::ip::udp::socket m_DiscoverySocket;
        asio::steady_timer m_DiscoveryTimer;
        
        asio::ip::tcp::resolver m_Resolver;
        
        asio::ip::tcp::acceptor m_HighPriorityAcceptor;
        asio::ip::tcp::acceptor m_NormalPriorityAcceptor;
        asio::ip::tcp::acceptor m_LowPriorityAcceptor;
        
        asio::ip::tcp::socket m_HighPrioritySocket;
        asio::ip::tcp::socket m_NormalPrioritySocket;
        asio::ip::tcp::socket m_LowPrioritySocket;
        asio::ip::address m_ListenAddress;
        
        asio::ip::address m_RemoteAddress;
    };
    
} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_REMOTE_CONTROL_SERVER_H