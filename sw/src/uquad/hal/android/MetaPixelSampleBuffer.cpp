#include "MetaPixelSampleBuffer.h"
#include "../../base/Error.h"

namespace uquad
{
namespace hal
{
namespace android
{
    MetaPixelSampleBuffer::MetaPixelSampleBuffer(shared_ptr<::android::encoder_media_buffer_type> const &mb, Vec2i const &size, base::ePixelFormat fmt)
        : m_MetaBuffer(mb)
        , m_Size(size)
        , m_Format(fmt)
        , m_LockCounter(0)
        , m_Data(0)
    {
        init();
    }

    MetaPixelSampleBuffer::~MetaPixelSampleBuffer()
    {
        release();
    }

    bool MetaPixelSampleBuffer::isValid() const
    {
        return m_MetaBuffer &&
            m_MetaBuffer->meta_handle->numFds == 1 &&
            m_MetaBuffer->meta_handle->numInts == 2;
    }

    void* MetaPixelSampleBuffer::data()
    {
        return m_Data;
    }

    std::size_t MetaPixelSampleBuffer::size() const
    {
        return m_MetaBuffer->meta_handle->data[2];
    }

    system::error_code MetaPixelSampleBuffer::lock()
    {
        if(!isValid())
            return base::makeErrorCode(base::kEInvalidContent);

        if(m_LockCounter++)
        {
            return base::makeErrorCode(base::kENoError);
        }

        m_Data = reinterpret_cast<uint8_t*>(::mmap(0,
            m_MetaBuffer->meta_handle->data[2],
            PROT_READ|PROT_WRITE, MAP_SHARED,
            m_MetaBuffer->meta_handle->data[0],
            0
        ));

        if(m_Data == MAP_FAILED)
        {
            m_LockCounter--;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code MetaPixelSampleBuffer::unlock()
    {
        if(!isValid())
            return base::makeErrorCode(base::kEInvalidContent);

        if(!m_LockCounter)
        {
            return base::makeErrorCode(base::kENotApplicable);
        }

        if(--m_LockCounter)
        {
            return base::makeErrorCode(base::kENoError);
        }

        if(::munmap(m_Data, m_MetaBuffer->meta_handle->data[2]) != 0)
        {
            m_LockCounter++;
            return base::makeErrorCode(base::kEInvalidContent);
        }

        m_Data = 0;
        return base::makeErrorCode(base::kENoError);
    }

    base::ePixelFormat MetaPixelSampleBuffer::format() const
    {
        return m_Format;
    }

    std::size_t MetaPixelSampleBuffer::planeCount() const
    {
        return m_PlaneCount;
    }

    Vec2i MetaPixelSampleBuffer::planeSize(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return Vec2i(0,0);

        return Vec2i(
            m_Size[0] >> m_PlaneSizeShift[pi][0],
            m_Size[1] >> m_PlaneSizeShift[pi][1]
        );
    }

    void* MetaPixelSampleBuffer::planeData(std::size_t pi)
    {
        if(!isValid() || !m_Data || pi >= m_PlaneCount)
            return 0;

        return m_Data + m_PlaneDataOffset[pi];
    }

    std::size_t MetaPixelSampleBuffer::planeRowBytes(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return 0;

        return m_PlaneRowBytes[pi];
    }

    void MetaPixelSampleBuffer::init()
    {
        switch(m_Format)
        {
        case base::kPixelFormatRGB8:
            {
                std::size_t const stride = m_Size[0];
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = stride*3;
            }
            break;

        case base::kPixelFormatBGRA8:
            {
                std::size_t const stride = m_Size[0];
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = stride*4;
            }
            break;

        case base::kPixelFormatYUV420sp:
            {
                std::size_t const regSize = static_cast<std::size_t>(m_Size[0]*m_Size[1]*1.5);

                std::size_t const stride = m_Size[0];
                m_PlaneCount = 2;

                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneSizeShift[1] = Vec2i(1,1);
                m_PlaneDataOffset[0] = 0;
                if(regSize == m_MetaBuffer->meta_handle->data[2])
                {
                    m_PlaneDataOffset[1] = m_Size[1]*stride;
                }
                else
                {
                    m_PlaneDataOffset[1] = (m_Size[1]*stride + 2047) & (~(2047));
                }
                m_PlaneRowBytes[0] = stride;
                m_PlaneRowBytes[1] = stride;
            }
            break;

        default: break;
        }
    }

    void MetaPixelSampleBuffer::release()
    {
        if(m_Data)
        {
            ::munmap(m_Data, m_MetaBuffer->meta_handle->data[2]);
        }
        m_Data = 0;
        m_LockCounter = 0;
        m_MetaBuffer.reset();
    }

} //namespace android
} //namespace hal
} //namespace uquad
