#ifndef UQUAD_MEDIA_SAMPLE_FORMAT_H
#define UQUAD_MEDIA_SAMPLE_FORMAT_H

#include "Config.h"
#include "SampleType.h"


namespace uquad
{
namespace media
{
    class SampleFormat
    {
    public:
        SampleFormat();
        SampleFormat(SampleType t, float r);
        
        
        SampleType sampleType;
        float sampleRate;
    };
    
    
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_SAMPLE_FORMAT_H