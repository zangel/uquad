#include "AVFramePixelSampleBuffer.h"
#include "../../base/Error.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
    AVFramePixelSampleBuffer::AVFramePixelSampleBuffer()
    {
        ::memset(&m_AVFrame, 0, sizeof(m_AVFrame));
    }
    
    AVFramePixelSampleBuffer::~AVFramePixelSampleBuffer()
    {
        release();
    }
    
    bool AVFramePixelSampleBuffer::create(base::ePixelFormat fmt, Vec2i const &size)
    {
        release();
        ::AVPixelFormat av_format = FFMpeg::avPixelFormatFromPixelFormat(fmt);
        if(av_format == AV_PIX_FMT_NONE)
            return false;
        
        ::avcodec_get_frame_defaults(&m_AVFrame);
        m_AVFrame.format = av_format;
        m_AVFrame.width = size(0);
        m_AVFrame.height = size(1);
        
        if(::av_image_alloc(
            m_AVFrame.data,
            m_AVFrame.linesize,
            m_AVFrame.width,
            m_AVFrame.height,
            av_format,
            32) < 0)
        {
            return false;
        }
        
        return true;
    }
    
    void AVFramePixelSampleBuffer::release()
    {
        if(m_AVFrame.data[0])
        {
            ::av_freep(&m_AVFrame.data[0]);
            ::memset(&m_AVFrame, 0, sizeof(m_AVFrame));
        }
    }
        
    bool AVFramePixelSampleBuffer::isValid() const
    {
        return m_AVFrame.format != ::AV_PIX_FMT_NONE && m_AVFrame.data[0] != 0;
    }
    
    void* AVFramePixelSampleBuffer::data()
    {
        return m_AVFrame.data[0];
    }
    
    std::size_t AVFramePixelSampleBuffer::size() const
    {
        return m_AVFrame.data[0] ? ::av_image_get_buffer_size(::AVPixelFormat(m_AVFrame.format), m_AVFrame.width, m_AVFrame.height, 32) : 0;
    }
    
    system::error_code AVFramePixelSampleBuffer::lock()
    {
        return base::makeErrorCode(isValid() ? base::kENoError : base::kEInvalidContent);
    }
    
    system::error_code AVFramePixelSampleBuffer::unlock()
    {
        return base::makeErrorCode(isValid() ? base::kENoError : base::kEInvalidContent);
    }
    
    base::ePixelFormat AVFramePixelSampleBuffer::format() const
    {
        if(!isValid())
            return base::kPixelFormatInvalid;
        
        return FFMpeg::pixelFormatFromAVPixelFormat(::AVPixelFormat(m_AVFrame.format));
    }
    
    std::size_t AVFramePixelSampleBuffer::planeCount() const
    {
        return
            (m_AVFrame.data[0]?1:0) +
            (m_AVFrame.data[1]?1:0) +
            (m_AVFrame.data[2]?1:0) +
            (m_AVFrame.data[3]?1:0);
    }
    
    Vec2i AVFramePixelSampleBuffer::planeSize(std::size_t pi) const
    {
        if(!isValid() || pi > 3 || !m_AVFrame.data[pi])
            return Vec2i(0,0);
        
        const AVPixFmtDescriptor *pix_desc = ::av_pix_fmt_desc_get(::AVPixelFormat(m_AVFrame.format));
        
        return Vec2i(
            m_AVFrame.width  >> pix_desc[pi].log2_chroma_w,
            m_AVFrame.height >> pix_desc[pi].log2_chroma_h
        );
    }
    
    void* AVFramePixelSampleBuffer::planeData(std::size_t pi)
    {
        if(pi > 3)
            return 0;
        
        return m_AVFrame.data[pi];
    }
    
    std::size_t AVFramePixelSampleBuffer::planeRowBytes(std::size_t pi) const
    {
        if(pi > 3)
            return 0;
        
        return m_AVFrame.linesize[pi];
    }
    
} //namespace ffmpeg
} //namespace media
} //namespace uquad