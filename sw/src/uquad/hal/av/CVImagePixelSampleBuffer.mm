#include "CVImagePixelSampleBuffer.h"
#include "../../base/Error.h"

namespace uquad
{
namespace hal
{
namespace av
{
    CVImagePixelSampleBuffer::CVImagePixelSampleBuffer()
        : m_ImgBufferRef(0)
    {
        time = base::PixelSampleBuffer::time_clock_t::now();
    }
    
    CVImagePixelSampleBuffer::CVImagePixelSampleBuffer(CVImageBufferRef ibr)
        : m_ImgBufferRef(ibr)
    {
        time = base::PixelSampleBuffer::time_clock_t::now();
        if(m_ImgBufferRef)
        {
            CVBufferRetain(m_ImgBufferRef);
        }
    }
    
    CVImagePixelSampleBuffer::~CVImagePixelSampleBuffer()
    {
        if(m_ImgBufferRef)
        {
            CVBufferRelease(m_ImgBufferRef);
        }
    }
    
    
    bool CVImagePixelSampleBuffer::isValid() const
    {
        return m_ImgBufferRef != 0;
    }
    
    void* CVImagePixelSampleBuffer::data()
    {
        return m_ImgBufferRef ? CVPixelBufferGetBaseAddress(m_ImgBufferRef) : 0;
    }
    
    std::size_t CVImagePixelSampleBuffer::size() const
    {
        return m_ImgBufferRef ? CVPixelBufferGetDataSize(m_ImgBufferRef) : 0;
    }
    
    system::error_code CVImagePixelSampleBuffer::lock()
    {
        if(!m_ImgBufferRef)
            return base::makeErrorCode(base::kEInvalidContent);
        
        return base::makeErrorCode(CVPixelBufferLockBaseAddress(m_ImgBufferRef, 0) == kCVReturnSuccess ? base::kENoError : base::kEInvalidContent);
    }
    
    system::error_code CVImagePixelSampleBuffer::unlock()
    {
        if(!m_ImgBufferRef)
            return base::makeErrorCode(base::kEInvalidContent);
        
        return base::makeErrorCode(CVPixelBufferUnlockBaseAddress(m_ImgBufferRef, 0) == kCVReturnSuccess ? base::kENoError: base::kEInvalidContent);
    }
    
    
    base::ePixelFormat CVImagePixelSampleBuffer::format() const
    {
        if(!m_ImgBufferRef)
            return base::kPixelFormatInvalid;
        
        return uquadFromOSXPixelFormat(CVPixelBufferGetPixelFormatType(m_ImgBufferRef));
    }
    
    std::size_t CVImagePixelSampleBuffer::planeCount() const
    {
        if(!m_ImgBufferRef)
            return 0;
        
        std::size_t nPlanes = CVPixelBufferGetPlaneCount(m_ImgBufferRef);
        return std::max(nPlanes, std::size_t(1));
    }
    
    Vec2i CVImagePixelSampleBuffer::planeSize(std::size_t pi) const
    {
        if(!m_ImgBufferRef)
            return Vec2i(0,0);
        
        if(CVPixelBufferGetPlaneCount(m_ImgBufferRef) > 1)
        {
            return Vec2i(
                CVPixelBufferGetWidthOfPlane(m_ImgBufferRef, pi),
                CVPixelBufferGetHeightOfPlane(m_ImgBufferRef, pi)
            );
        }
        
        if(pi != 0)
            return Vec2i(0,0);
        
        return Vec2i(
            CVPixelBufferGetWidth(m_ImgBufferRef),
            CVPixelBufferGetHeight(m_ImgBufferRef)
        );
    }
    
    void* CVImagePixelSampleBuffer::planeData(std::size_t pi)
    {
        if(!m_ImgBufferRef)
            return 0;
        
        if(CVPixelBufferGetPlaneCount(m_ImgBufferRef) > 1)
            return CVPixelBufferGetBaseAddressOfPlane(m_ImgBufferRef, pi);
        
        if(pi != 0)
            return 0;
        
        return CVPixelBufferGetBaseAddress(m_ImgBufferRef);
    }
    
    std::size_t CVImagePixelSampleBuffer::planeRowBytes(std::size_t pi) const
    {
        if(!m_ImgBufferRef)
            return 0;
        
        if(CVPixelBufferGetPlaneCount(m_ImgBufferRef) > 1)
            return CVPixelBufferGetBytesPerRowOfPlane(m_ImgBufferRef, pi);
        
        if(pi != 0)
            return 0;
        
        return CVPixelBufferGetBytesPerRow(m_ImgBufferRef);
    }
    
    base::ePixelFormat CVImagePixelSampleBuffer::uquadFromOSXPixelFormat(int fmt)
    {
        switch (fmt)
        {
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
            return base::kPixelFormatYUV420sp;
            break;
        
        case kCVPixelFormatType_32BGRA:
            return base::kPixelFormatBGRA8;
            break;

        default:
            break;
        }
        return base::kPixelFormatInvalid;
    }

    int CVImagePixelSampleBuffer::osxFromUQuadPixelFormat(base::ePixelFormat fmt)
    {
        switch (fmt)
        {
        case base::kPixelFormatYUV420sp:
            return kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            break;
        
        case base::kPixelFormatBGRA8:
            return kCVPixelFormatType_32BGRA;
            break;

        default:
            break;
        }
        return -1;
    }
    
} //namespace av
} //namespace hal
} //namespace uquad