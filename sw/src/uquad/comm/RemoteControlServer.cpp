#include "RemoteControlServer.h"
#include "../base/Error.h"
#include "Log.h"

namespace uquad
{
namespace comm
{

    RemoteControlServer::RemoteControlServer(intrusive_ptr<base::Runloop> rl)
        : RemoteControl(rl)
        , m_DiscoverySocket(rl->ios())
        , m_DiscoveryTimer(rl->ios())
        , m_Resolver(rl->ios())
        , m_HighPriorityAcceptor(rl->ios())
        , m_NormalPriorityAcceptor(rl->ios())
        , m_LowPriorityAcceptor(rl->ios())
        , m_HighPrioritySocket(rl->ios())
        , m_NormalPrioritySocket(rl->ios())
        , m_LowPrioritySocket(rl->ios())
        , m_ListenAddress(asio::ip::address::from_string("0.0.0.0"))
    {
    }
    
    RemoteControlServer::~RemoteControlServer()
    {
    }
    
    asio::ip::address RemoteControlServer::getRemoteAddress() const
    {
        return m_RemoteAddress;
    }
    
    void RemoteControlServer::doStart(asio::yield_context yctx, handler_t handler)
    {
        if(!startDiscovery(yctx))
        {
            UQUAD_COMM_LOG(Warning) << "RemoteControlServer::doStart: failed to start discovery!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(!startListening(yctx))
        {
            UQUAD_COMM_LOG(Warning) << "RemoteControlServer::doStart: failed to start listening on sockets!";
            stopDiscovery(yctx);
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        m_State = kStateStarted;
        handler(base::makeErrorCode(base::kENoError));
        asio::spawn(m_Strand, bind(&RemoteControlServer::doAccept, this, _1));
        notifyOnRemoteControlStarted();
        UQUAD_COMM_LOG(Debug) << "RemoteControlServer::doStart: started successfully!";
    }
    
    void RemoteControlServer::doStop(asio::yield_context yctx, handler_t handler)
    {
        if(!isStarted())
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::doStop: not started!";
            handler(base::makeErrorCode(base::kENotStarted));
            return;
        }
        
        if(isConnected())
        {
            m_HighPriorityChannel.close();
            m_NormalPriorityChannel.close();
            m_LowPriorityChannel.close();
            
            m_State = kStateStarted;
            notifyOnRemoteControlDisconnected();
        }
        
        stopListening(yctx);
        stopDiscovery(yctx);
        
        m_State = kStateIdle;
        handler(base::makeErrorCode(base::kENoError));
        notifyOnRemoteControlStopped();
        UQUAD_COMM_LOG(Debug) << "RemoteControlServer::doStop: stopped!";
    }
    
    bool RemoteControlServer::startDiscovery(asio::yield_context &yctx)
    {
        system::error_code err;
        asio::ip::udp::endpoint discoveryEP(asio::ip::udp::v4(), 0);
        m_DiscoverySocket.open(discoveryEP.protocol(), err);
        
        if(err)
            return false;
        
        m_DiscoverySocket.set_option(asio::ip::udp::socket::reuse_address(false));
        m_DiscoverySocket.set_option(asio::ip::multicast::enable_loopback(true));
        m_DiscoverySocket.set_option(asio::socket_base::broadcast(true));
        
        m_DiscoverySocket.bind(discoveryEP, err);
        if(err)
        {
            m_DiscoverySocket.close();
            return false;
        }
        
        m_DiscoveryTimer.expires_from_now(std::chrono::milliseconds(DISCOVERY_INTERVAL_MS));
        m_DiscoveryTimer.async_wait(
            bind(
                &RemoteControlServer::handleDiscoveryTimer, this,
                _1
            )
        );
        
        return true;
    }
    
    void RemoteControlServer::stopDiscovery(asio::yield_context &yctx)
    {
        m_DiscoveryTimer.cancel();
        
        m_DiscoverySocket.cancel();
        m_DiscoverySocket.close();
    }
    
    bool RemoteControlServer::startListening(asio::yield_context &yctx)
    {
        if(!startListening(yctx, m_HighPriorityAcceptor, HIGH_PRIORITY_CHANNEL_PORT))
        {
            return false;
        }
        
        if(!startListening(yctx, m_NormalPriorityAcceptor, NORMAL_PRIORITY_CHANNEL_PORT))
        {
            m_HighPriorityAcceptor.close();
            return false;
        }
        
        if(!startListening(yctx, m_LowPriorityAcceptor, LOW_PRIORITY_CHANNEL_PORT))
        {
            m_HighPriorityAcceptor.close();
            m_NormalPriorityAcceptor.close();
            return false;
        }
        
        return true;
    }
    
    bool RemoteControlServer::startListening(asio::yield_context &yctx, asio::ip::tcp::acceptor &a, int p)
    {
        system::error_code err;
        asio::ip::tcp::resolver::query listenQuery(m_ListenAddress.to_string(), boost::lexical_cast<std::string>(p));
        asio::ip::tcp::resolver::iterator itEndpoint = m_Resolver.async_resolve(listenQuery, yctx[err]);
        
        if(err || itEndpoint == asio::ip::tcp::resolver::iterator())
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::startListening: failed to resolve host:port, e=" << err;
            return false;
        }
            
        
        asio::ip::tcp::endpoint listenEndpoint = *itEndpoint;
        
        a.open(listenEndpoint.protocol(), err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::startListening: acceptor.open failed, e=" << err;
            return false;
        }
        
        a.set_option(asio::ip::tcp::acceptor::reuse_address(true), err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::startListening: acceptor.set_option failed, e=" << err;
            a.close(err);
            return false;
        }
        
        a.bind(listenEndpoint, err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::startListening: acceptor.bind failed, e=" << err;
            a.close(err);
            return false;
        }
        
        a.listen(boost::asio::socket_base::max_connections, err);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::startListening: acceptor.listen failed, e=" << err;
            a.close(err);
            return false;
        }
        
        return true;
    }
    
    void RemoteControlServer::stopListening(asio::yield_context &yctx)
    {
        stopListening(yctx, m_HighPriorityAcceptor);
        stopListening(yctx, m_NormalPriorityAcceptor);
        stopListening(yctx, m_LowPriorityAcceptor);
    }
    
    void RemoteControlServer::stopListening(asio::yield_context &yctx, asio::ip::tcp::acceptor &a)
    {
        a.cancel();
        a.close();
    }
    
    void RemoteControlServer::doAccept(asio::yield_context yctx)
    {
        if(!acceptChannels(yctx))
        {
            return;
        }
        m_State = kStateConnected;
        notifyOnRemoteControlConnected();
        startReceivingMessages();
    }
    
    bool RemoteControlServer::acceptChannels(asio::yield_context &yctx)
    {
        if(m_HighPriorityChannel.isOpen() || !acceptChannel(yctx, m_HighPriorityChannel, m_HighPriorityAcceptor, m_HighPrioritySocket))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::acceptChannels: failed to accept high priority channel!";
            return false;
        }
        
        if(m_NormalPriorityChannel.isOpen() || !acceptChannel(yctx, m_NormalPriorityChannel, m_NormalPriorityAcceptor, m_NormalPrioritySocket))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::acceptChannels: failed to accept normal priority channel!";
            m_HighPriorityChannel.close();
            return false;
        }
        
        if(m_LowPriorityChannel.isOpen() || !acceptChannel(yctx, m_LowPriorityChannel, m_LowPriorityAcceptor, m_LowPrioritySocket))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::acceptChannels: failed to accept low priority channel!";
            m_NormalPriorityChannel.close();
            m_HighPriorityChannel.close();
            return false;
        }
        
        UQUAD_COMM_LOG(Debug) << "RemoteControlServer::acceptChannels: channels accepted";
        return true;
    }
    
    bool RemoteControlServer::acceptChannel(asio::yield_context &yctx, Channel &c, asio::ip::tcp::acceptor &a, asio::ip::tcp::socket &s)
    {
        system::error_code err;
        a.async_accept(s, yctx[err]);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlServer::acceptChannel: failed to accept, e=" << err;
            return false;
        }
        
        s.set_option(asio::ip::tcp::no_delay(true), err);
        m_RemoteAddress = s.remote_endpoint().address();
        c.setSocket(std::move(s));
        
        return true;
    }
    
    bool RemoteControlServer::handleReceiveMessageError(system::error_code e)
    {
        if(e == asio::error::operation_aborted)
        {
            return false;
        }
        
        if(isConnected())
        {
            m_HighPriorityChannel.close();
            m_NormalPriorityChannel.close();
            m_LowPriorityChannel.close();
            
            m_State = kStateStarted;
            notifyOnRemoteControlDisconnected();
            asio::spawn(m_Strand, bind(&RemoteControlServer::doAccept, this, _1));
            UQUAD_COMM_LOG(Debug) << "RemoteControlServer::handleReceiveMessageError: channels disconnected!";
        }
        return false;
    }
        
    void RemoteControlServer::handleDiscoveryTimer(system::error_code e)
    {
        if(e)
        {
        }
        else
        {
            m_DiscoveryTimer.expires_from_now(std::chrono::milliseconds(DISCOVERY_INTERVAL_MS));
            m_DiscoveryTimer.async_wait(
                bind(
                    &RemoteControlServer::handleDiscoveryTimer, this,
                    _1
                )
            );
            
            m_DiscoverySocket.async_send_to(
                asio::buffer(&DISCOVERY_MAGIC, sizeof(DISCOVERY_MAGIC)),
                asio::ip::udp::endpoint(
                    DISCOVERY_MULTICAST_ADDRESS,
                    DISCOVERY_PORT
                ),
                bind(
                    &RemoteControlServer::handleBroadcastDiscovery, this,
                    _1,
                    _2
                )
            );
        }
    }
    
    void RemoteControlServer::handleBroadcastDiscovery(system::error_code e, std::size_t n)
    {
        if(e)
        {
            UQUAD_COMM_LOG(Warning) << "RemoteControlServer::handleBroadcastDiscovery: failed to broadcast discovery msg: " << e;
        }
    }
    
} //namespace comm
} //namespace uquad
