#ifndef UQUAD_MEDIA_VIDEO_SAMPLE_FORMAT_H
#define UQUAD_MEDIA_VIDEO_SAMPLE_FORMAT_H

#include "Config.h"
#include "SampleFormat.h"


namespace uquad
{
namespace media
{
    class VideoSampleFormat
        : public SampleFormat
    {
    public:
        VideoSampleFormat();
        VideoSampleFormat(SampleType t, float r, Vec2i s);
        
        Vec2i sampleSize;
    };
    
    
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VIDEO_SAMPLE_FORMAT_H