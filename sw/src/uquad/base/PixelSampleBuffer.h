#ifndef UQUAD_BASE_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_BASE_PIXEL_SAMPLE_BUFFER_H

#include "Config.h"
#include "PixelBuffer.h"
#include "SampleBuffer.h"

namespace uquad
{
namespace base
{
	class PixelSampleBuffer
    	: public PixelBuffer
        , public SampleBuffer
    {
    public:
    	PixelSampleBuffer();
    	virtual ~PixelSampleBuffer();
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_PIXEL_SAMPLE_BUFFER_H
