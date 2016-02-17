#ifndef UQUAD_MEDIA_CONSUMER_H
#define UQUAD_MEDIA_CONSUMER_H

#include "Config.h"
#include "Sink.h"

namespace uquad
{
namespace media
{
    class Consumer
    	: public Sink
    {
    public:
        Consumer();
    	~Consumer();
        
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CONSUMER_H
