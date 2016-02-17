#ifndef UQUAD_MEDIA_SAMPLE_H
#define UQUAD_MEDIA_SAMPLE_H

#include "Config.h"
#include "../base/SampleBuffer.h"

namespace uquad
{
namespace media
{
    struct Sample
    {
        Sample()
            : buffer()
            , time(0.0f)
        {
        }
        
        Sample(intrusive_ptr<base::SampleBuffer> const &b, float t)
            : buffer(b)
            , time(t)
        {
        }
        
        
        inline operator bool() const { return buffer.get(); }
        
        intrusive_ptr<base::SampleBuffer> buffer;
        float time;
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_SAMPLE_H
