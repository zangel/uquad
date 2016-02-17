#ifndef UQUAD_MEDIA_WRITER_H
#define UQUAD_MEDIA_WRITER_H

#include "Config.h"
#include "Consumer.h"
#include "ContainerFormat.h"
#include "VideoSampleFormat.h"
#include "../base/IOStream.h"


namespace uquad
{
namespace media
{
    class Writer
    	: public Consumer
    {
    public:
        
        class Registry
            : public Component::Registry
        {
        public:
            Registry(std::string const &n);
            ~Registry();
            
            virtual eContainerFormat getContainerFormat() const = 0;
        };
        
    	Writer();
    	~Writer();
        
        virtual eContainerFormat getContainerFormat() const = 0;
        virtual VideoSampleFormat const& getVideoSampleFormat() const = 0;
        
        virtual intrusive_ptr<base::IOStream> getStream() const = 0;
        virtual system::error_code setStream(intrusive_ptr<base::IOStream> const &stm) = 0;

    protected:
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_WRITER_H
