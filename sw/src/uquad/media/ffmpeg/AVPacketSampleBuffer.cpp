#include "AVPacketSampleBuffer.h"
#include "../../base/Error.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
    AVPacketSampleBuffer::AVPacketSampleBuffer()
    {
        init();
    }
    
    AVPacketSampleBuffer::~AVPacketSampleBuffer()
    {
        release();
    }
    
    bool AVPacketSampleBuffer::init()
    {
        ::av_init_packet(&m_AVPacket);
        return true;
    }
    
    void AVPacketSampleBuffer::release()
    {
        ::av_free_packet(&m_AVPacket);
    }
        
    bool AVPacketSampleBuffer::isValid() const
    {
        return m_AVPacket.data != 0;;
    }
    
    void* AVPacketSampleBuffer::data()
    {
        return m_AVPacket.data;
    }
    
    std::size_t AVPacketSampleBuffer::size() const
    {
        return m_AVPacket.size;
    }
    
    system::error_code AVPacketSampleBuffer::lock()
    {
        return base::makeErrorCode(isValid() ? base::kENoError : base::kEInvalidContent);
    }
    
    system::error_code AVPacketSampleBuffer::unlock()
    {
        return base::makeErrorCode(isValid() ? base::kENoError : base::kEInvalidContent);
    }
    
} //namespace ffmpeg
} //namespace media
} //namespace uquad