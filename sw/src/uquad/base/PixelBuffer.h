#ifndef UQUAD_BASE_PIXEL_BUFFER_H
#define UQUAD_BASE_PIXEL_BUFFER_H

#include "Config.h"
#include "Buffer.h"
#include "PixelFormat.h"

namespace uquad
{
namespace base
{
	class PixelBuffer
    	: public virtual Buffer
    {
    public:
    	PixelBuffer();
    	virtual ~PixelBuffer();
        
        virtual ePixelFormat format() const = 0;
        virtual std::size_t planeCount() const = 0;
        
        virtual Vec2i planeSize(std::size_t pi = 0) const = 0;
        virtual void* planeData(std::size_t pi = 0) = 0;
        virtual std::size_t planeRowBytes(std::size_t pi = 0) const = 0;
        
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_SAMPLE_BUFFER_H
