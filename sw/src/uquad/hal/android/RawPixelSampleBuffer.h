#ifndef UQUAD_HAL_ANDROID_RAW_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_HAL_ANDROID_RAW_PIXEL_SAMPLE_BUFFER_H

#include "../Config.h"
#include "../../base/PixelSampleBuffer.h"


namespace uquad
{
namespace hal
{
namespace android
{
    class RawPixelSampleBuffer
        : public base::PixelSampleBuffer
    {
    public:
        RawPixelSampleBuffer(intrusive_ptr<base::Buffer> const &stg, Vec2i const &size, base::ePixelFormat fmt);
        ~RawPixelSampleBuffer();

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

    private:
        void init();


    private:
        intrusive_ptr<base::Buffer> m_Storage;
        Vec2i m_Size;
        base::ePixelFormat m_Format;

        std::size_t m_PlaneCount;
        Vec2i m_PlaneSizeShift[2];
        std::size_t m_PlaneDataOffset[2];
        std::size_t m_PlaneRowBytes[2];
    };

} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_RAW_PIXEL_SAMPLE_BUFFER_H
