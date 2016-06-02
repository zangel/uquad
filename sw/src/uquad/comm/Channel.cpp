#include "Channel.h"
#include "Message.h"
#include "../base/Error.h"
#include "Log.h"

namespace uquad
{
namespace comm
{
    Channel::SocketDevice::SocketDevice(Channel &ch)
        : channel(ch)
    {
    }
    
    std::streamsize Channel::SocketDevice::read(char_type *s, std::streamsize n)
    {
        if(!channel.m_ReceiveYield)
            return 0;
        
        system::error_code err;
        std::streamsize nr = channel.m_Socket.get().async_read_some(asio::buffer(s, n), (*channel.m_ReceiveYield)[err]);
        asio::detail::throw_error(err);
        return nr;
    }
    
    std::streamsize Channel::SocketDevice::write(const char_type* s, std::streamsize n)
    {
        if(!channel.m_SendYield)
            return 0;
        
        system::error_code err;
        
        std::streamsize nw = 0;
        while(n)
        {
            std::streamsize inw = channel.m_Socket.get().async_write_some(asio::buffer(s + nw, n), (*channel.m_SendYield)[err]);
            if(err) break;
            n -= inw;
            nw += inw;
        }
        asio::detail::throw_error(err);
        return nw;
    }
    
    Channel::Channel(asio::io_service &ios)
        : m_IOS(ios)
        , m_ReceiveStrand(m_IOS)
        , m_SendStrand(m_IOS)
        , m_SocketStreamBuf(ref(*this))
        , m_Socket()
        , m_ReceiveYield(0)
        , m_SendYield(0)
        , m_SendBufferQueueSync()
        , m_SendBufferQueue(2)
    {
    }
    
    Channel::~Channel()
    {
        close();
    }

    void Channel::setSocket(asio::ip::tcp::socket s)
    {
        close();
        m_Socket = std::move(s);
    }
    
    bool Channel::isOpen() const
    {
        return m_Socket && m_Socket.get().is_open();
    }
    
    void Channel::close()
    {
        if(m_Socket)
        {
            if(m_Socket.get().is_open())
            {
                system::error_code err;
                m_Socket.get().cancel(err);
                m_Socket.get().shutdown(asio::ip::tcp::socket::shutdown_both, err);
                m_Socket.get().close(err);
            }
            m_Socket.reset();
            m_ReceiveYield = 0;
            m_SendYield = 0;
        }
    }
    
    void Channel::asyncReceiveMessage(handler_t handler)
    {
        asio::spawn(m_ReceiveStrand, bind(&Channel::doReceiveMessage, this, _1, std::move(handler)));
    }
    
    void Channel::asyncSendMessage(intrusive_ptr<Message> msg, handler_t handler)
    {
        lock_guard<fast_mutex> lock(m_SendBufferQueueSync);
        if(m_SendBufferQueue.full())
        {
            handler(std::move(msg), base::makeErrorCode(base::kEDeviceBusy));
        }
        else
        {
            bool shouldSpawn = m_SendBufferQueue.empty();
            m_SendBufferQueue.push_back(std::make_pair(std::move(handler), std::move(msg)));
            if(shouldSpawn)
            {
                asio::spawn(m_SendStrand, bind(&Channel::doSendMessage, this, _1));
            }
        }
    }
    
    void Channel::doReceiveMessage(asio::yield_context yield, handler_t handler)
    {
        BOOST_ASSERT(!m_ReceiveYield);
        
        if(!m_Socket)
        {
            handler(intrusive_ptr<Message>(), base::makeErrorCode(base::kENotOpened));
            return;
        }
        
        
        m_ReceiveYield = &yield;
        base::IArchive archive(m_SocketStreamBuf);
        intrusive_ptr<Message> msg;
        
        try
        {
            archive & msg;
            m_SocketStreamBuf.pubsync();
            m_ReceiveYield = 0;
            handler(std::move(msg), base::makeErrorCode(base::kENoError));
        }
        catch(system::system_error e)
        {
            m_ReceiveYield = 0;
            if(isOpen())
                handler(intrusive_ptr<Message>(), e.code());
            UQUAD_COMM_LOG(Error) << "Channel::doReceiveMessage: exception occurred: " << e.code() << ", " << e.what();
        }
        catch(...)
        {
            m_ReceiveYield = 0;
            if(isOpen())
                handler(intrusive_ptr<Message>(), base::makeErrorCode(base::kEInvalidContent));
        }
    }
    
    void Channel::doSendMessage(asio::yield_context yield)
    {
        BOOST_ASSERT(!m_SendYield);
        m_SendYield = &yield;
        
        while(!m_SendBufferQueue.empty())
        {
            handler_t handler;
            intrusive_ptr<Message> msg;
            handler = m_SendBufferQueue.front().first;
            msg = m_SendBufferQueue.front().second;
            
            if(isOpen())
            {
                base::OArchive archive(m_SocketStreamBuf);
                
                try
                {
                    archive & msg;
                    m_SocketStreamBuf.pubsync();
                    handler(std::move(msg), base::makeErrorCode(base::kENoError));
                }
                catch(system::system_error e)
                {
                    handler(std::move(msg), e.code());
                    UQUAD_COMM_LOG(Error) << "Channel::doSendMessage: exception occurred: " << e.code() << ", " << e.what();
                }
                catch(...)
                {
                    handler(std::move(msg), base::makeErrorCode(base::kEInvalidContent));
                }
            }
            else
            {
                handler(std::move(msg), base::makeErrorCode(base::kENotOpened));
            }
            
            lock_guard<fast_mutex> lock(m_SendBufferQueueSync);
            m_SendBufferQueue.pop_front();
        }
        
        m_SendYield = 0;
    }
    
} //namespace comm
} //namespace uquad
