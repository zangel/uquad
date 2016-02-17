#ifndef UQUAD_HAL_AV_CV_IMAGE_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_HAL_AV_CV_IMAGE_PIXEL_SAMPLE_BUFFER_H

#include "../Config.h"
#include "../../base/PixelSampleBuffer.h"
#include <AVFoundation/AVFoundation.h>

namespace uquad
{
namespace hal
{
namespace av
{
    class CVImagePixelSampleBuffer
        : public base::PixelSampleBuffer
    {
    public:
        CVImagePixelSampleBuffer();
        CVImagePixelSampleBuffer(CVImageBufferRef ibr);
        ~CVImagePixelSampleBuffer();
        
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
        
        inline CVImageBufferRef imageBuffer() { return m_ImgBufferRef; }
        
        static base::ePixelFormat uquadFromOSXPixelFormat(int fmt);
        static int osxFromUQuadPixelFormat(base::ePixelFormat fmt);
        
        
    private:
        CVImageBufferRef m_ImgBufferRef;
        
    };
    
} //namespace av
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_AV_CV_IMAGE_PIXEL_SAMPLE_BUFFER_H