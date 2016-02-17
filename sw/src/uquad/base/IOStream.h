#ifndef UQUAD_BASE_IO_STREAM_H
#define UQUAD_BASE_IO_STREAM_H

#include "Config.h"
#include "RefCounted.h"

namespace uquad
{
namespace base
{
	class IOStream
    	: public RefCounted
    {
    public:
        IOStream();
        ~IOStream();
        
        virtual system::error_code open(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) = 0;
        virtual bool isOpen() const = 0;
        virtual void close() = 0;
        
        virtual std::size_t read(void *buff, std::size_t n) = 0;
        virtual std::size_t write(void const *buff, std::size_t n) = 0;
    };
    
    class IOAsyncStream
        : public IOStream
    {
    public:
        typedef boost::function<void(system::error_code, std::size_t)> Handler;
        
        IOAsyncStream();
        ~IOAsyncStream();
        
        virtual void asyncRead(void *buff, std::size_t n, Handler handler) = 0;
        virtual void asyncWrite(void const *buff, std::size_t n, Handler handler) = 0;
        
    };
    
} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_IO_STREAM_H
