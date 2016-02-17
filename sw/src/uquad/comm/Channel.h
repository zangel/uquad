#ifndef UQUAD_COMM_CHANNEL_H
#define UQUAD_COMM_CHANNEL_H

#include "Config.h"
#include "../base/RefCounted.h"

namespace uquad
{
namespace comm
{
    class Message;
    
    class Channel
        : public base::RefCounted
    {
        class SocketDevice
        {
        public:
            typedef char char_type;
            
            typedef ios::bidirectional_device_tag category;
            
            SocketDevice(Channel &ch);
            
            std::streamsize read(char_type *s, std::streamsize n);
            std::streamsize write(const char_type* s, std::streamsize n);
            
            Channel &channel;
        };
        
        typedef ios::stream_buffer<SocketDevice> SocketStreamBuf;
        friend class SocketDevice;
        
    public:
        typedef function<void (intrusive_ptr<Message>, system::error_code)> handler_t;
        
        Channel(asio::io_service &ios);
        ~Channel();

        void setSocket(asio::ip::tcp::socket s);
        bool isOpen() const;
        void close();
        
        void asyncReceiveMessage(handler_t handler);
        void asyncSendMessage(intrusive_ptr<Message> msg, handler_t handler);
        
    protected:
        void doReceiveMessage(asio::yield_context yield, handler_t handler);
        void doSendMessage(asio::yield_context yield);
        
    private:
        typedef std::pair< handler_t, intrusive_ptr<Message> > handler_msg_pair_t;
        asio::io_service &m_IOS;
        asio::io_service::strand m_ReceiveStrand;
        asio::io_service::strand m_SendStrand;
        SocketStreamBuf m_SocketStreamBuf;
        optional<asio::ip::tcp::socket> m_Socket;
        asio::yield_context *m_ReceiveYield;
        asio::yield_context *m_SendYield;
        
        fast_mutex m_SendBufferQueueSync;
        circular_buffer<handler_msg_pair_t> m_SendBufferQueue;
    };
    
} //namespace comm
} //namespace uquad

#endif //UQUAD_COMM_CHANNEL_H