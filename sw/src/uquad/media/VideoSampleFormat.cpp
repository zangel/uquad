#include "VideoSampleFormat.h"

namespace uquad
{
namespace media
{
    VideoSampleFormat::VideoSampleFormat()
        : SampleFormat()
        , sampleSize(0,0)
    {
    }
    
    VideoSampleFormat::VideoSampleFormat(SampleType t, float r, Vec2i s)
        : SampleFormat(t, r)
        , sampleSize(s)
    {
    }

} //namespace media
} //namespace uquad