#ifndef UQUAD_HAL_ANDROID_CAMERA_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_HAL_ANDROID_CAMERA_PIXEL_SAMPLE_BUFFER_H

#include "../Config.h"
#include "../../base/PixelSampleBuffer.h"
#include <system/graphics.h>
#include <hardware/gralloc.h>


namespace uquad
{
namespace hal
{
namespace android
{
    class CameraPixelSampleBuffer
        : public base::PixelSampleBuffer
    {
    public:
        CameraPixelSampleBuffer();
        CameraPixelSampleBuffer(Vec2i const &size, int format, int usage);
        ~CameraPixelSampleBuffer();

        enum
        {
            CREATE_USAGE =
                GRALLOC_USAGE_SW_READ_OFTEN |
                GRALLOC_USAGE_SW_WRITE_NEVER |
                GRALLOC_USAGE_HW_TEXTURE |
                GRALLOC_USAGE_HW_VIDEO_ENCODER |
                GRALLOC_USAGE_HW_CAMERA_WRITE |
                GRALLOC_USAGE_HW_CAMERA_READ,
            LOCK_USAGE =
                GRALLOC_USAGE_SW_READ_OFTEN
        };

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

        buffer_handle_t handle() { return m_Handle; }
        buffer_handle_t* handlePtr() { return &m_Handle; }
        int stride() { return m_Stride; }

        system::error_code create(Vec2i const &size, int format, int usage);
        void release();

    private:
        buffer_handle_t m_Handle;
        int m_Stride;
        int m_Format;
        int m_Usage;
        Vec2i m_Size;

        atomic<uint32_t> m_LockCounter;
        uint8_t *m_Data;
        std::size_t m_DataSize;

        std::size_t m_PlaneCount;
        Vec2i m_PlaneSizeShift[2];
        std::size_t m_PlaneDataOffset[2];
        std::size_t m_PlaneRowBytes[2];
    };

} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_CAMERA_PIXEL_SAMPLE_BUFFER_H
