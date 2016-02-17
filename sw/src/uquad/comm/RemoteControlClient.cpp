#include "RemoteControlClient.h"
#include "../base/Error.h"
#include "Log.h"

namespace uquad
{
namespace comm
{
    RemoteControlClient::RemoteControlClient(intrusive_ptr<base::Runloop> rl)
        : RemoteControl(rl)
        , m_bRemoteDiscovered(false)
        , m_DiscoverySocket(rl->ios())
        , m_WatchdogTimer(rl->ios())
        , m_Resolver(rl->ios())
        , m_DiscoveryMsgBuffer(0)
    {
    }
    
    RemoteControlClient::~RemoteControlClient()
    {
    }
    
    asio::ip::address RemoteControlClient::getRemoteAddress() const
    {
        return m_RemoteDiscoveryEndpoint.address();
    }
    
    void RemoteControlClient::doStart(asio::yield_context yctx, handler_t handler)
    {
        if(!startDiscovery(yctx))
            handler(base::makeErrorCode(base::kENotStarted));
        
        m_State = kStateStarted;
        handler(base::makeErrorCode(base::kENoError));
        notifyOnRemoteControlStarted();
        UQUAD_COMM_LOG(Debug) << "RemoteControlClient::doStart: started successfully!";
    }
    
    void RemoteControlClient::doStop(asio::yield_context yctx, handler_t handler)
    {
        if(!isStarted())
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::doStop: not started!";
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
        
        stopDiscovery(yctx);
        
        m_State = kStateIdle;
        handler(base::makeErrorCode(base::kENoError));
        notifyOnRemoteControlStopped();
        UQUAD_COMM_LOG(Debug) << "RemoteControlClient::doStop: stopped!";
    }
    
    bool RemoteControlClient::startDiscovery(asio::yield_context &yctx)
    {
        m_bRemoteDiscovered = false;
        
        system::error_code err;
        
        asio::ip::udp::endpoint discoveryEP(asio::ip::udp::v4(), DISCOVERY_PORT);
        m_DiscoverySocket.open(discoveryEP.protocol());
        m_DiscoverySocket.set_option(asio::ip::udp::socket::reuse_address(true));
        m_DiscoverySocket.bind(discoveryEP, err);
        if(err)
        {
            m_DiscoverySocket.close();
            return false;
        }
        
        m_DiscoverySocket.set_option(asio::ip::multicast::join_group(DISCOVERY_MULTICAST_ADDRESS));
        m_DiscoverySocket.set_option(asio::ip::multicast::enable_loopback(true));
        m_DiscoverySocket.set_option(asio::ip::multicast::hops(32));
        
        m_DiscoverySocket.async_receive_from(
            asio::buffer(&m_DiscoveryMsgBuffer, sizeof(m_DiscoveryMsgBuffer)),
            m_RemoteDiscoveryEndpoint,
            bind(
                &RemoteControlClient::handleDiscoveryMessage, this,
                _1,
                _2
            )
        );
        
        return true;
    }
    
    void RemoteControlClient::stopDiscovery(asio::yield_context &yctx)
    {
        m_WatchdogTimer.cancel();
        m_DiscoverySocket.cancel();
        m_DiscoverySocket.close();
    }
    
    
    void RemoteControlClient::handleDiscoveryMessage(system::error_code e, std::size_t n)
    {
        if(e)
        {
            UQUAD_COMM_LOG(Warning) << "RemoteControlClient::handleDiscoveryMessage: failed to receive discovery msg: " << e;
        }
        else
        {
            m_DiscoverySocket.async_receive_from(
                asio::buffer(&m_DiscoveryMsgBuffer, sizeof(m_DiscoveryMsgBuffer)),
                m_RemoteDiscoveryEndpoint,
                bind(
                    &RemoteControlClient::handleDiscoveryMessage, this,
                    _1,
                    _2
                )
            );
            
            if(!m_bRemoteDiscovered)
            {
                m_bRemoteDiscovered = true;
                if(!isConnected())
                {
                    asio::spawn(m_Strand, bind(&RemoteControlClient::doConnect, this, _1));
                }
            }
            
            m_WatchdogTimer.cancel();
            m_WatchdogTimer.expires_from_now(std::chrono::milliseconds(2*DISCOVERY_INTERVAL_MS));
            m_WatchdogTimer.async_wait(
                bind(
                    &RemoteControlClient::handleWatchdogTimer, this,
                    _1
                )
            );
            
        }
    }
    
    void RemoteControlClient::handleWatchdogTimer(system::error_code e)
    {
        if(e)
        {
            if(e != asio::error::operation_aborted)
            {
                UQUAD_COMM_LOG(Warning) << "RemoteControlClient::handleWatchdogTimer: watchdog timer error: " << e;
            }
        }
        else
        {
            UQUAD_COMM_LOG(Warning) << "RemoteControlClient::handleWatchdogTimer: watchdog timer expired";
        }
    }
    
    void RemoteControlClient::doConnect(asio::yield_context yctx)
    {
        if(!connectChannels(yctx))
        {
            return;
        }
        
        m_State = kStateConnected;
        notifyOnRemoteControlConnected();
        startReceivingMessages();
    }
    
    
    bool RemoteControlClient::connectChannels(asio::yield_context &yctx)
    {
        if(m_HighPriorityChannel.isOpen() || !connectChannel(yctx, m_HighPriorityChannel, HIGH_PRIORITY_CHANNEL_PORT))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::connectChannels: failed to connect high priority channel!";
            return false;
        }
        
        if(m_NormalPriorityChannel.isOpen() || !connectChannel(yctx, m_NormalPriorityChannel, NORMAL_PRIORITY_CHANNEL_PORT))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::connectSockets: failed to connect normal priority channel!";
            m_HighPriorityChannel.close();
            return false;
        }
        
        if(m_LowPriorityChannel.isOpen() || !connectChannel(yctx, m_LowPriorityChannel, LOW_PRIORITY_CHANNEL_PORT))
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::connectSockets: failed to connect low priority channel!";
            m_NormalPriorityChannel.close();
            m_HighPriorityChannel.close();
            return false;
        }
        
        UQUAD_COMM_LOG(Debug) << "RemoteControlClient::connectChannels: channels connected";
        
        return true;
    }
    
    bool RemoteControlClient::connectChannel(asio::yield_context &yctx, Channel &c, int p)
    {
        asio::ip::tcp::socket socket(m_Runloop->ios());
        
        system::error_code err;
        
        asio::ip::tcp::resolver::query hostQuery(m_RemoteDiscoveryEndpoint.address().to_string(), boost::lexical_cast<std::string>(p));
        asio::ip::tcp::resolver::iterator itEndpoint = m_Resolver.async_resolve(hostQuery, yctx[err]);
        
        if(err || itEndpoint == asio::ip::tcp::resolver::iterator())
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::connectChannel: failed to resolve host:port, e=" << err;
            return false;
        }
        
        socket.async_connect(*itEndpoint, yctx[err]);
        if(err)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControlClient::connectChannel: failed to connect, e=" << err;
            return false;
        }
        
        c.setSocket(std::move(socket));
        return true;
    }
        
    bool RemoteControlClient::handleReceiveMessageError(system::error_code e)
    {
        if(isConnected())
        {
            m_HighPriorityChannel.close();
            m_NormalPriorityChannel.close();
            m_LowPriorityChannel.close();
            
            m_State = kStateStarted;
            notifyOnRemoteControlDisconnected();
            asio::spawn(m_Strand, bind(&RemoteControlClient::doConnect, this, _1));
            UQUAD_COMM_LOG(Debug) << "RemoteControlClient::handleReceiveMessageError: channels disconnected!";
        }
        return false;
    }
    

        
    
} //namespace comm
} //namespace uquad