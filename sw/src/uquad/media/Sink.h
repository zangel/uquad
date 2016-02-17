#ifndef UQUAD_MEDIA_SINK_H
#define UQUAD_MEDIA_SINK_H

#include "Config.h"
#include "Component.h"
#include "Port.h"

namespace uquad
{
namespace media
{
    class Sink
    	: public virtual Component
    {
    public:
    	Sink();
    	~Sink();
        
        virtual std::size_t numInputPorts() const = 0;
        virtual intrusive_ptr<InputPort> getInputPort(std::size_t index) const = 0;
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_SINK_H
