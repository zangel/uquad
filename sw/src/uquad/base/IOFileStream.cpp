#include "IOFileStream.h"
#include "Error.h"

namespace uquad
{
namespace base
{
    IOFileStream::IOFileStream()
        : m_Path()
        , m_Handle(FileHandle::s_InvalidNativeHandle)
    {
    }
    
    IOFileStream::IOFileStream(fs::path const &path)
        : m_Path(path)
        , m_Handle(FileHandle::s_InvalidNativeHandle)
    {
    }
    
    IOFileStream::~IOFileStream()
    {
        close();
    }
    
    system::error_code IOFileStream::setPath(fs::path const &path)
    {
        if(isOpen())
            return makeErrorCode(kEAlreadyOpened);
        
        m_Path = path;
        return makeErrorCode(kENoError);
    }
    
    system::error_code IOFileStream::open(std::ios_base::openmode mode)
    {
        if(isOpen())
            return makeErrorCode(kEAlreadyOpened);
        
        if(m_Path.empty())
            return makeErrorCode(kENotApplicable);
        
        int oflag = 0;
        if((mode & (std::ios_base::in | std::ios_base::out)) == (std::ios_base::in | std::ios_base::out))
        {
            if(mode & std::ios_base::app)
                return makeErrorCode(kEInvalidArgument);
            
            oflag |= O_RDWR;
            
            if(mode & std::ios_base::trunc)
            {
                oflag |= O_TRUNC;
                oflag |= O_CREAT;
            }
        }
        else
        if(mode & std::ios_base::in)
        {
            if(mode & (std::ios_base::app | std::ios_base::trunc))
                return makeErrorCode(kEInvalidArgument);
            oflag |= O_RDONLY;
        }
        else
        if(mode & std::ios_base::out)
        {
            if((mode & (std::ios_base::app | std::ios_base::trunc)) == (std::ios_base::app | std::ios_base::trunc))
                return makeErrorCode(kEInvalidArgument);
            
            oflag |= O_WRONLY;
            if(mode & std::ios_base::app)
                oflag |= O_APPEND;
            else
            {
                oflag |= O_CREAT;
                oflag |= O_TRUNC;
            }
        }
        else
        {
            return makeErrorCode(kEInvalidArgument);
        }
        
        mode_t pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        
        int desc = ::open(m_Path.string().c_str(), oflag, pmode);

        if(desc < 0)
        {
            return makeErrorCode(kENotOpened);
        }
        
        m_Handle.setNativeHandle(desc);
        return makeErrorCode(kENoError);
    }
    
    bool IOFileStream::isOpen() const
    {
        return m_Handle.isValid();
    }
    
    void IOFileStream::close()
    {
        if(!isOpen())
            return;
        
        m_Handle.close();
    }
    
    std::size_t IOFileStream::read(void *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        return ::read(m_Handle.getNativeHandle(), buff, n);
    }
    
    std::size_t IOFileStream::write(void const *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        return ::write(m_Handle.getNativeHandle(), buff, n);
    }
    
    IOAsyncFileStream::IOAsyncFileStream(intrusive_ptr<Runloop> const &rl)
        : m_Runloop(rl)
        , m_Path()
        , m_Stream(m_Runloop->ios())
    {
    }
    
    IOAsyncFileStream::IOAsyncFileStream(intrusive_ptr<Runloop> const &rl, fs::path const &path)
        : m_Runloop(rl)
        , m_Path(path)
        , m_Stream(m_Runloop->ios())
    {
    }
    
    IOAsyncFileStream::~IOAsyncFileStream()
    {
        close();
    }
    
    system::error_code IOAsyncFileStream::setPath(fs::path const &path)
    {
        if(isOpen())
            return makeErrorCode(kEAlreadyOpened);
        
        m_Path = path;
        return makeErrorCode(kENoError);
    }
    
    system::error_code IOAsyncFileStream::open(std::ios_base::openmode mode)
    {
        if(isOpen())
            return makeErrorCode(kEAlreadyOpened);
        
        if(m_Path.empty())
            return makeErrorCode(kENotApplicable);
        
        int oflag = 0;
        if((mode & (std::ios_base::in | std::ios_base::out)) == (std::ios_base::in | std::ios_base::out))
        {
            if(mode & std::ios_base::app)
                return makeErrorCode(kEInvalidArgument);
            
            oflag |= O_RDWR;
            
            if(mode & std::ios_base::trunc)
            {
                oflag |= O_TRUNC;
                oflag |= O_CREAT;
            }
        }
        else
        if(mode & std::ios_base::in)
        {
            if(mode & (std::ios_base::app | std::ios_base::trunc))
                return makeErrorCode(kEInvalidArgument);
            oflag |= O_RDONLY;
        }
        else
        if(mode & std::ios_base::out)
        {
            if((mode & (std::ios_base::app | std::ios_base::trunc)) == (std::ios_base::app | std::ios_base::trunc))
                return makeErrorCode(kEInvalidArgument);
            
            oflag |= O_WRONLY;
            if(mode & std::ios_base::app)
                oflag |= O_APPEND;
            else
            {
                oflag |= O_CREAT;
                oflag |= O_TRUNC;
            }
        }
        else
        {
            return makeErrorCode(kEInvalidArgument);
        }
        
        mode_t pmode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        
        int desc = ::open(m_Path.string().c_str(), oflag, pmode);

        if(desc < 0)
        {
            return makeErrorCode(kENotOpened);
        }
        
        m_Stream.assign(desc);
        return makeErrorCode(kENoError);
    }
    
    bool IOAsyncFileStream::isOpen() const
    {
        return m_Stream.is_open();
    }
    
    void IOAsyncFileStream::close()
    {
        if(!isOpen())
            return;
        
        m_Stream.cancel();
        stream_descriptor::native_handle_type h = m_Stream.native_handle();
        m_Stream.close();
        ::close(h);
    }
    
    std::size_t IOAsyncFileStream::read(void *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        return asio::read(m_Stream, asio::buffer(buff, n));
    }
    
    std::size_t IOAsyncFileStream::write(void const *buff, std::size_t n)
    {
        if(!isOpen())
            return 0;
        
        return asio::write(m_Stream, asio::buffer(buff, n));
    }
    
    void IOAsyncFileStream::asyncRead(void *buff, std::size_t n, IOAsyncStream::Handler handler)
    {
        asio::async_read(m_Stream, asio::buffer(buff, n), std::move(handler));
    }
    
    void IOAsyncFileStream::asyncWrite(void const *buff, std::size_t n, IOAsyncStream::Handler handler)
    {
        asio::async_write(m_Stream, asio::buffer(buff, n), std::move(handler));
    }


} //namespace base
} //namespace uquad
