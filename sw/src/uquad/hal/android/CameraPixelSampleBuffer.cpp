#include "CameraPixelSampleBuffer.h"
#include "../base/Error.h"
#include "GRAlloc.h"

namespace uquad
{
namespace hal
{
namespace android
{
    CameraPixelSampleBuffer::CameraPixelSampleBuffer()
        : m_Handle(0)
        , m_Stride(0)
        , m_Format(base::kPixelFormatInvalid)
        , m_Size(0,0)
        , m_LockCounter(0)
        , m_Data(0)
        , m_DataSize(0)
        , m_PlaneCount(0)
    {

    }

    CameraPixelSampleBuffer::CameraPixelSampleBuffer(Vec2i const &size, int format, int usage)
        : m_Handle(0)
        , m_Stride(0)
        , m_Format(0)
        , m_Usage(0)
        , m_Size(0,0)
        , m_LockCounter(0)
        , m_Data(0)
        , m_DataSize(0)
        , m_PlaneCount(0)
    {
        create(size, format, usage);
    }

    CameraPixelSampleBuffer::~CameraPixelSampleBuffer()
    {
        release();
    }

    bool CameraPixelSampleBuffer::isValid() const
    {
        return m_Handle != 0;
    }

    void* CameraPixelSampleBuffer::data()
    {
        return m_Data;
    }

    std::size_t CameraPixelSampleBuffer::size() const
    {
        return m_DataSize;
    }

    system::error_code CameraPixelSampleBuffer::lock()
    {
        if(!isValid())
            return base::makeErrorCode(base::kEInvalidContent);

        if(m_LockCounter++)
        {
            return base::makeErrorCode(base::kENoError);
        }

        //lock the buffer
        if(SI(GRAlloc).lock(m_Handle, LOCK_USAGE, m_Size, reinterpret_cast<void**>(&m_Data)))
        {
            m_LockCounter--;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code CameraPixelSampleBuffer::unlock()
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

        //unlock the buffer
        if(SI(GRAlloc).unlock(m_Handle))
        {
            m_LockCounter++;

        }

        m_Data = 0;

        return base::makeErrorCode(base::kENoError);
    }

    base::ePixelFormat CameraPixelSampleBuffer::format() const
    {
        return GRAlloc::pixelFormatFromHalPixelFormat(m_Format);
    }

    std::size_t CameraPixelSampleBuffer::planeCount() const
    {
        return m_PlaneCount;
    }

    Vec2i CameraPixelSampleBuffer::planeSize(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return Vec2i(0,0);

        return Vec2i(
            m_Size[0] >> m_PlaneSizeShift[pi][0],
            m_Size[1] >> m_PlaneSizeShift[pi][1]
        );
    }

    void* CameraPixelSampleBuffer::planeData(std::size_t pi)
    {
        if(!isValid() || !m_Data || pi >= m_PlaneCount)
            return 0;

        return m_Data + m_PlaneDataOffset[pi];
    }

    std::size_t CameraPixelSampleBuffer::planeRowBytes(std::size_t pi) const
    {
        if(!isValid() || pi >= m_PlaneCount)
            return 0;

        return m_PlaneRowBytes[pi];
    }

    system::error_code CameraPixelSampleBuffer::create(Vec2i const &size, int format, int usage)
    {
        if(isValid())
            return base::makeErrorCode(base::kEAlreadyOpened);

        if(system::error_code ae = SI(GRAlloc).alloc(size, format, usage, &m_Handle, &m_Stride))
            return ae;

        m_Format = format;
        m_Usage = usage;
        m_Size = size;

        switch(this->format())
        {
        case base::kPixelFormatRGB8:
            {
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = m_Stride*3;
            }
            break;

        case base::kPixelFormatBGRA8:
            {
                m_PlaneCount = 1;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneDataOffset[0] = 0;
                m_PlaneRowBytes[0] = m_Stride*4;
            }
            break;

        case base::kPixelFormatYUV420sp:
            {
                m_PlaneCount = 2;
                m_PlaneSizeShift[0] = Vec2i(0,0);
                m_PlaneSizeShift[1] = Vec2i(1,1);
                m_PlaneDataOffset[0] = 0;
                m_PlaneDataOffset[1] = m_Size(1)*m_Stride;
                m_PlaneRowBytes[0] = m_Stride;
                m_PlaneRowBytes[1] = m_Stride;
            }
            break;

        default: break;
        }


        return base::makeErrorCode(base::kENoError);
    }

    void CameraPixelSampleBuffer::release()
    {
        if(isValid())
        {
            SI(GRAlloc).free(m_Handle);
            m_Handle = 0;
            m_Stride = 0;
            m_Format = 0;
            m_Usage = 0;
            m_Size = Vec2i(0,0);
            m_LockCounter = 0;
            m_Data = 0;
            m_DataSize = 0;
            m_PlaneCount = 0;
        }
    }


} //namespace android
} //namespace hal
} //namespace uquad
