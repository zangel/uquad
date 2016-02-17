#ifndef UQUAD_MEDIA_FFMPEG_AVPACKET_SAMPLE_BUFFER_H
#define UQUAD_MEDIA_FFMPEG_AVPACKET_SAMPLE_BUFFER_H

#include "../Config.h"
#include "FFMpeg.h"
#include "../../base/SampleBuffer.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
    class AVPacketSampleBuffer
        : public base::SampleBuffer
    {
    public:
        AVPacketSampleBuffer();
        ~AVPacketSampleBuffer();
        
        bool init();
        void release();
        
        bool isValid() const;
        void* data();
        std::size_t size() const;
        
        system::error_code lock();
        system::error_code unlock();
        
        ::AVPacket m_AVPacket;
    };
    
} //namespace mac
} //namespace hal
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_AVPACKET_SAMPLE_BUFFER_H