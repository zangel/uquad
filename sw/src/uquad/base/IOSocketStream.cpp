#include "IOSocketStream.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    IOAsyncSocketStream::IOAsyncSocketStream()
        : m_Socket()
    {
    }
    
    IOAsyncSocketStream::IOAsyncSocketStream(asio::ip::tcp::socket s)
        : m_Socket()
    {
        m_Socket = std::move(s);
    }
    
    IOAsyncSocketStream::~IOAsyncSocketStream()
    {
        close();
    }
    
    void IOAsyncSocketStream::setSocket(asio::ip::tcp::socket s)
    {
        close();
        m_Socket = std::move(s);
    }
    
    system::error_code IOAsyncSocketStream::open(std::ios_base::openmode mode)
    {
        if(isOpen())
            return makeErrorCode(kENoError);
        
        return base::makeErrorCode(base::kENotApplicable);
    }
    
    bool IOAsyncSocketStream::isOpen() const
    {
        return m_Socket && m_Socket.get().is_open();
    }
    
    void IOAsyncSocketStream::close()
    {
        if(!m_Socket || !m_Socket.get().is_open())
            return;
        
        system::error_code err;
        
        m_Socket.get().cancel(err);
        m_Socket.get().shutdown(asio::ip::tcp::socket::shutdown_both, err);
        m_Socket.get().close(err);
        m_Socket.reset();
    }
    
    std::size_t IOAsyncSocketStream::read(void *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        system::error_code re;
        std::size_t nread = m_Socket.get().read_some(asio::buffer(buff, n), re);
        if(re && re != boost::asio::error::operation_aborted)
        {
            notifyOnIOAsyncSocketStreamError(re);
        }
        return nread;
    }
    
    std::size_t IOAsyncSocketStream::write(void const *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        system::error_code we;
        std::size_t nwritten = m_Socket.get().write_some(asio::buffer(buff, n), we);
        if(we && we != boost::asio::error::operation_aborted)
        {
            notifyOnIOAsyncSocketStreamError(we);
        }
        return nwritten;
    }
    
    void IOAsyncSocketStream::asyncRead(void *buff, std::size_t n, IOAsyncStream::Handler handler)
    {
        if(!isOpen())
            return;
        
        m_Socket.get().async_read_some(
            asio::buffer(buff, n),
            boost::bind(
                &IOAsyncSocketStream::handleIOHandler, this, std::move(handler),
                _1, _2
            )
        );
    }
    
    void IOAsyncSocketStream::asyncWrite(void const *buff, std::size_t n, IOAsyncStream::Handler handler)
    {
        if(!isOpen())
            return;
        
        m_Socket.get().async_write_some(
            asio::buffer(buff, n),
            boost::bind(
                &IOAsyncSocketStream::handleIOHandler, this, std::move(handler),
                _1, _2
            )
        );
    }
    
    void IOAsyncSocketStream::handleIOHandler(IOAsyncStream::Handler handler, system::error_code err, std::size_t bytes)
    {
        if(isOpen())
        {
            handler(err, bytes);
            if(err && err != boost::asio::error::operation_aborted)
            {
                notifyOnIOAsyncSocketStreamError(err);
            }
        }
    }
    


} //namespace base
} //namespace uquad
