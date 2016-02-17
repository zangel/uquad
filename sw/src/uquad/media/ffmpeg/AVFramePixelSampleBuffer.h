#ifndef UQUAD_MEDIA_FFMPEG_AVFRAME_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_MEDIA_FFMPEG_AVFRAME_PIXEL_SAMPLE_BUFFER_H

#include "../Config.h"
#include "FFMpeg.h"
#include "../../base/PixelSampleBuffer.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
    class AVFramePixelSampleBuffer
        : public base::PixelSampleBuffer
    {
    public:
        AVFramePixelSampleBuffer();
        ~AVFramePixelSampleBuffer();
        
        bool create(base::ePixelFormat fmt, Vec2i const &size);
        void release();
        
        bool isValid() const;
        void* data();
        std::size_t size() const;
        
        system::error_code lock();
        system::error_code unlock();
        
        base::ePixelFormat format() const;
        std::size_t planeCount() const;
        
        Vec2i planeSize(std::size_t pi = 0) const;
        void* planeData(std::size_t pi = 0);
        std::size_t planeRowBytes(std::size_t pi = 0) const;
        
        ::AVFrame m_AVFrame;
    };
    
} //namespace mac
} //namespace hal
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_AVFRAME_PIXEL_SAMPLE_BUFFER_H