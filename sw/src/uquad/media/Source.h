#ifndef UQUAD_MEDIA_SOURCE_H
#define UQUAD_MEDIA_SOURCE_H

#include "Config.h"
#include "Component.h"
#include "Port.h"

namespace uquad
{
namespace media
{
    class Source
    	: public virtual Component
    {
    public:
    	Source();
    	~Source();
        
        virtual std::size_t numOutputPorts() const = 0;
        virtual intrusive_ptr<OutputPort> getOutputPort(std::size_t index) const = 0;
        
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_SOURCE_H
