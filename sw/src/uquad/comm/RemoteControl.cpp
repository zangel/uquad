#include "RemoteControl.h"
#include "Log.h"

namespace uquad
{
namespace comm
{
#if defined(UQUAD_PLATFORM_Android)
    asio::ip::address RemoteControl::DISCOVERY_MULTICAST_ADDRESS = asio::ip::address::from_string("192.168.0.255");
#else
    asio::ip::address RemoteControl::DISCOVERY_MULTICAST_ADDRESS = asio::ip::address::from_string("239.255.255.250");
#endif


    
    int const RemoteControl::DISCOVERY_INTERVAL_MS = 500;
    uint32_t const RemoteControl::DISCOVERY_MAGIC = 'UQDI';
        
    
    RemoteControl::RemoteControl(intrusive_ptr<base::Runloop> rl)
        : base::RefCounted()
        , m_State(kStateIdle)
        , m_Runloop(rl)
        , m_Strand(m_Runloop->ios())
        , m_HighPriorityChannel(m_Runloop->ios())
        , m_NormalPriorityChannel(m_Runloop->ios())
        , m_LowPriorityChannel(m_Runloop->ios())
    {
    }
    
    RemoteControl::~RemoteControl()
    {
    }
    
    
    void RemoteControl::start(handler_t handler)
    {
        asio::spawn(m_Strand, bind(&RemoteControl::doStart, this, _1, std::move(handler)));
    }
    
    void RemoteControl::stop(handler_t handler)
    {
        asio::spawn(m_Strand, bind(&RemoteControl::doStop, this, _1, std::move(handler)));
    }
    
    void RemoteControl::sendMessage(intrusive_ptr<Message> msg, Channel::handler_t handler)
    {
        switch(msg->priority())
        {
        case Message::kPriorityHigh:
            m_HighPriorityChannel.asyncSendMessage(msg, std::move(handler));
            break;
            
        case Message::kPriorityNormal:
            m_NormalPriorityChannel.asyncSendMessage(msg, std::move(handler));
            break;
            
        case Message::kPriorityLow:
            m_LowPriorityChannel.asyncSendMessage(msg, std::move(handler));
            break;
            
        default:
            break;
        }
        
    }
    
    void RemoteControl::startReceivingMessages()
    {
        receiveHighPriorityMessage();
        receiveNormalPriorityMessage();
        receiveLowPriorityMessage();
    }
    
    void RemoteControl::receiveHighPriorityMessage()
    {
        m_HighPriorityChannel.asyncReceiveMessage(
            bind(
                &RemoteControl::handleHighPriorityMessageReceived, this,
                _1,
                _2
            )
        );
    }
    
    void RemoteControl::receiveNormalPriorityMessage()
    {
        m_NormalPriorityChannel.asyncReceiveMessage(
            bind(
                &RemoteControl::handleNormalPriorityMessageReceived, this,
                _1,
                _2
            )
        );
    }
    
    void RemoteControl::receiveLowPriorityMessage()
    {
        m_LowPriorityChannel.asyncReceiveMessage(
            bind(
                &RemoteControl::handleLowPriorityMessageReceived, this,
                _1,
                _2
            )
        );
    }
    
    void RemoteControl::handleHighPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e)
    {
        if(e)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControl::handleHighPriorityMessageReceived: io error: " << e;
            if(handleReceiveMessageError(e))
                receiveHighPriorityMessage();
        }
        else
        {
            notifyOnRemoteControlMessageReceived(std::move(msg));
            receiveHighPriorityMessage();
        }
    }
    
    void RemoteControl::handleNormalPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e)
    {
        if(e)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControl::handleNormalPriorityMessageReceived: io error: " << e;
            if(handleReceiveMessageError(e))
                receiveNormalPriorityMessage();
        }
        else
        {
            notifyOnRemoteControlMessageReceived(std::move(msg));
            receiveNormalPriorityMessage();
        }
    }
    
    void RemoteControl::handleLowPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e)
    {
        if(e)
        {
            UQUAD_COMM_LOG(Error) << "RemoteControl::handleLowPriorityMessageReceived: io error: " << e;
            if(handleReceiveMessageError(e))
                receiveLowPriorityMessage();
        }
        else
        {
            notifyOnRemoteControlMessageReceived(std::move(msg));
            receiveLowPriorityMessage();
        }
    }

    
} //namespace comm
} //namespace uquad
