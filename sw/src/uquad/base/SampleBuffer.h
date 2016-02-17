#ifndef UQUAD_BASE_SAMPLE_BUFFER_H
#define UQUAD_BASE_SAMPLE_BUFFER_H

#include "Config.h"
#include "Buffer.h"

namespace uquad
{
namespace base
{
	class SampleBuffer
    	: public virtual Buffer
    {
    public:
        typedef chrono::steady_clock time_clock_t;
        
    	SampleBuffer();
    	virtual ~SampleBuffer();
        
        virtual system::error_code lock() = 0;
        virtual system::error_code unlock() = 0;
        
        time_clock_t::time_point time;
    };

} //namespace base
} //namespace uquad


#endif //UQUAD_BASE_SAMPLE_BUFFER_H
