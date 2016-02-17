#ifndef UQUAD_BASE_IO_FILE_STREAM_H
#define UQUAD_BASE_IO_FILE_STREAM_H

#include "Config.h"
#include "IOStream.h"
#include "Runloop.h"
#include "FileHandle.h"

namespace uquad
{
namespace base
{
    class IOFileStream
    	: public IOStream
    {
    public:
        IOFileStream();
        IOFileStream(fs::path const &path);
        ~IOFileStream();
        
        
        inline fs::path const& getPath() const { return m_Path; }
        system::error_code setPath(fs::path const &path);
        
        system::error_code open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
        bool isOpen() const;
        void close();
        
        std::size_t read(void *buff, std::size_t n);
        std::size_t write(void const *buff, std::size_t n);
        
    private:
        fs::path m_Path;
        FileHandle m_Handle;
    };
    
	class IOAsyncFileStream
    	: public IOAsyncStream
    {
    public:
        typedef asio::posix::stream_descriptor stream_descriptor;
        
        IOAsyncFileStream(intrusive_ptr<Runloop> const &rl);
        IOAsyncFileStream(intrusive_ptr<Runloop> const &rl, fs::path const &path);
        ~IOAsyncFileStream();
        
        
        inline fs::path const& getPath() const { return m_Path; }
        system::error_code setPath(fs::path const &path);
        
        system::error_code open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
        bool isOpen() const;
        void close();
        
        std::size_t read(void *buff, std::size_t n);
        std::size_t write(void const *buff, std::size_t n);
        
        void asyncRead(void *buff, std::size_t n, IOAsyncStream::Handler handler);
        void asyncWrite(void const *buff, std::size_t n, IOAsyncStream::Handler handler);
        
    private:
        intrusive_ptr<Runloop> m_Runloop;
        fs::path m_Path;
        stream_descriptor m_Stream;
    };
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_IO_FILE_STREAM_H
