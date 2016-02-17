#include "SampleFormat.h"

namespace uquad
{
namespace media
{
    
    SampleFormat::SampleFormat()
        : sampleType()
        , sampleRate(0.0f)
    {
    }
    
    SampleFormat::SampleFormat(SampleType t, float r)
        : sampleType(t)
        , sampleRate(r)
    {
    }
    
} //namespace media
} //namespace uquad