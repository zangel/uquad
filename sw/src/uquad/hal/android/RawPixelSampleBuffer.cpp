#include "RawPixelSampleBuffer.h"
#include "../../base/Error.h"

namespace uquad
{
namespace hal
{
namespace android
{
    RawPixelSampleBuffer::RawPixelSampleBuffer(intrusive_ptr<base::Buffer> const &stg, Vec2i const &size, base::ePixelFormat fmt)
        : m_Storage(stg)
        , m_Size(size)
        , m_Format(fmt)
    {
        init();
    }

    RawPixelSampleBuffer::~RawPixelSampleBuffer()
    {
    }

    bool RawPixelSampleBuffer::isValid() const
    {
        return m_Storage->isValid();
    }

    void* RawPixelSampleBuffer::data()
    {
        return m_Storage->data();
    }

    std::size_t RawPixelSampleBuffer::size() const
    {
        return m_Storage->size();
    }

    system::error_code RawPixelSampleBuffer::lock()
    {
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code RawPixelSampleBuffer::unlock()
    {
        return base::makeErrorCode(base::kENoError);
    }

    base::ePixelFormat RawPixelSampleBuffer::format() const
    {
        return m_Format;
    }

    std::size_t RawPixelSampleBuffer::planeCount() const
    {
        return m_PlaneCount;
    }

    Vec2i RawPixelSampleBuffer::planeSize(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return Vec2i(0,0);

        return Vec2i(
            m_Size[0] >> m_PlaneSizeShift[pi][0],
            m_Size[1] >> m_PlaneSizeShift[pi][1]
        );
    }

    void* RawPixelSampleBuffer::planeData(std::size_t pi)
    {
        if(!isValid() || pi >= m_PlaneCount)
            return 0;

        return reinterpret_cast<uint8_t*>(m_Storage->data()) + m_PlaneDataOffset[pi];
    }

    std::size_t RawPixelSampleBuffer::planeRowBytes(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return 0;

        return m_PlaneRowBytes[pi];
    }

    void RawPixelSampleBuffer::init()
    {
        switch(m_Format)
        {
        case base::kPixelFormatRGB8:
            {
                std::size_t stride = m_Size[0];
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = stride*3;
            }
            break;

        case base::kPixelFormatBGRA8:
            {
                std::size_t stride = m_Size[0];
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = stride*4;
            }
            break;

        case base::kPixelFormatYUV420sp:
            {
                std::size_t stride = m_Size[0];
                m_PlaneCount = 2;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneSizeShift[1] = Vec2i(1,1);
                m_PlaneDataOffset[0] = 0;
                m_PlaneDataOffset[1] = m_Size[1]*stride;
                m_PlaneRowBytes[0] = stride;
                m_PlaneRowBytes[1] = stride;
            }
            break;

        default: break;
        }
    }

} //namespace android
} //namespace hal
} //namespace uquad
