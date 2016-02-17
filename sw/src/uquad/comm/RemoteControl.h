#ifndef UQUAD_COMM_REMOTE_CONTROL_H
#define UQUAD_COMM_REMOTE_CONTROL_H

#include "Config.h"
#include "../base/RefCounted.h"
#include "../base/Delegate.h"
#include "../base/Runloop.h"
#include "Channel.h"
#include "Message.h"

namespace uquad
{
namespace comm
{
    class RemoteControlDelegate
	{
	public:
		RemoteControlDelegate() = default;
		virtual ~RemoteControlDelegate() = default;
        virtual void onRemoteControlStarted() {}
        virtual void onRemoteControlStopped() {}
        virtual void onRemoteControlConnected() {}
        virtual void onRemoteControlDisconnected() {}
        virtual void onRemoteControlMessageReceived(intrusive_ptr<Message>) {}
	};
    
    class RemoteControl
        : public base::RefCounted
    {
    public:
        static int const DISCOVERY_PORT                         = 35620;
        static int const HIGH_PRIORITY_CHANNEL_PORT             = 35621;
        static int const NORMAL_PRIORITY_CHANNEL_PORT           = 35622;
        static int const LOW_PRIORITY_CHANNEL_PORT              = 35623;
        
        static asio::ip::address DISCOVERY_MULTICAST_ADDRESS;
        static int const DISCOVERY_INTERVAL_MS;
        static uint32_t const DISCOVERY_MAGIC;
        
        typedef base::Delegate<RemoteControlDelegate> remote_control_delegate_t;
        
        typedef function<void (system::error_code)> handler_t;
        
        typedef enum
        {
            kStateIdle = 0,
            kStateStarted,
            kStateConnected
        } eState;
        
        
        RemoteControl(intrusive_ptr<base::Runloop> rl);
        ~RemoteControl();
        
        void start(handler_t handler);
        inline void start() { start([](system::error_code){}); }
        
        inline bool isStarted() const { return m_State >= kStateStarted; }
        inline bool isConnected() const { return m_State >= kStateConnected; }
        
        void stop(handler_t handler);
        inline void stop()  { stop([](system::error_code){}); }
        
        inline void sendMessage(intrusive_ptr<Message> msg) { sendMessage(msg, [](intrusive_ptr<Message>, system::error_code){} ); }
        void sendMessage(intrusive_ptr<Message> msg, Channel::handler_t handler);
        
        virtual asio::ip::address getRemoteAddress() const = 0;
        
        inline remote_control_delegate_t const& remoteControlDelegate() { return m_RemoteControlDelegate; }
        
    protected:
        inline void notifyOnRemoteControlStarted()
        {
            m_RemoteControlDelegate.call(&RemoteControlDelegate::onRemoteControlStarted);
        }
        
        inline void notifyOnRemoteControlStopped()
        {
            m_RemoteControlDelegate.call(&RemoteControlDelegate::onRemoteControlStopped);
        }
        
        inline void notifyOnRemoteControlConnected()
        {
            m_RemoteControlDelegate.call(&RemoteControlDelegate::onRemoteControlConnected);
        }
        
        inline void notifyOnRemoteControlDisconnected()
        {
            m_RemoteControlDelegate.call(&RemoteControlDelegate::onRemoteControlDisconnected);
        }
        
        inline void notifyOnRemoteControlMessageReceived(intrusive_ptr<Message> msg)
        {
            m_RemoteControlDelegate.call(&RemoteControlDelegate::onRemoteControlMessageReceived, std::move(msg));
        }
        
        void startReceivingMessages();
        
        virtual void doStart(asio::yield_context yctx, handler_t handler) = 0;
        virtual void doStop(asio::yield_context yctx, handler_t handler) = 0;
        
        virtual bool handleReceiveMessageError(system::error_code e) = 0;
        
    private:
        void receiveHighPriorityMessage();
        void receiveNormalPriorityMessage();
        void receiveLowPriorityMessage();
        
        void handleHighPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e);
        void handleNormalPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e);
        void handleLowPriorityMessageReceived(intrusive_ptr<Message> msg, system::error_code e);
        

        
    protected:
        remote_control_delegate_t m_RemoteControlDelegate;
        eState m_State;
        intrusive_ptr<base::Runloop> m_Runloop;
        asio::io_service::strand m_Strand;
        Channel m_HighPriorityChannel;
        Channel m_NormalPriorityChannel;
        Channel m_LowPriorityChannel;
        
    };
    
} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_REMOTE_CONTROL_H