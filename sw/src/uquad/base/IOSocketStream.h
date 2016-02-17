#ifndef UQUAD_BASE_IO_SOCKET_STREAM_H
#define UQUAD_BASE_IO_SOCKET_STREAM_H

#include "Config.h"
#include "IOStream.h"
#include "Runloop.h"
#include "Delegate.h"


namespace uquad
{
namespace base
{
    class IOAsyncSocketStreamDelegate
	{
	public:
		IOAsyncSocketStreamDelegate() = default;
		virtual ~IOAsyncSocketStreamDelegate() = default;
        
        virtual void onIOAsyncSocketStreamError(system::error_code const &) {}
	};
    
    
    class IOAsyncSocketStream
    	: public IOAsyncStream
    {
    public:
        typedef Delegate<IOAsyncSocketStreamDelegate> io_async_stream_delegate_t;
        
        IOAsyncSocketStream();
        IOAsyncSocketStream(asio::ip::tcp::socket s);
        ~IOAsyncSocketStream();
        
        void setSocket(asio::ip::tcp::socket s);
        
        system::error_code open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
        bool isOpen() const;
        void close();
        
        std::size_t read(void *buff, std::size_t n);
        std::size_t write(void const *buff, std::size_t n);
        
        void asyncRead(void *buff, std::size_t n, IOAsyncStream::Handler handler);
        void asyncWrite(void const *buff, std::size_t n, IOAsyncStream::Handler handler);
        
        inline io_async_stream_delegate_t const& ioAsyncStreamDelegate() { return m_IOAsyncStreamDelegate; }
        
    protected:
        inline void notifyOnIOAsyncSocketStreamError(system::error_code const &e)
        {
            m_IOAsyncStreamDelegate.call(&IOAsyncSocketStreamDelegate::onIOAsyncSocketStreamError, e);
        }
        
    private:
        void handleIOHandler(IOAsyncStream::Handler handler, system::error_code err, std::size_t bytes);
        
        
    private:
        io_async_stream_delegate_t m_IOAsyncStreamDelegate;
        optional<asio::ip::tcp::socket> m_Socket;
    };
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_IO_SOCKET_STREAM_H
