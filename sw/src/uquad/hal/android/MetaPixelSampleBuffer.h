#ifndef UQUAD_HAL_ANDROID_META_PIXEL_SAMPLE_BUFFER_H
#define UQUAD_HAL_ANDROID_META_PIXEL_SAMPLE_BUFFER_H

#include "../Config.h"
#include "../../base/PixelSampleBuffer.h"
#include <libstagefrighthw/QComOMXMetadata.h>


namespace uquad
{
namespace hal
{
namespace android
{
    class MetaPixelSampleBuffer
        : public base::PixelSampleBuffer
    {
    public:
        MetaPixelSampleBuffer(shared_ptr<::android::encoder_media_buffer_type> const &mb, Vec2i const &size, base::ePixelFormat fmt);
        ~MetaPixelSampleBuffer();

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

        inline ::android::encoder_media_buffer_type* encoderMediaBuffer() { return m_MetaBuffer.get(); }

    private:
        void init();
        void release();

    private:
        shared_ptr<::android::encoder_media_buffer_type> m_MetaBuffer;
        Vec2i m_Size;
        base::ePixelFormat m_Format;

        atomic<uint32_t> m_LockCounter;
        uint8_t *m_Data;

        std::size_t m_PlaneCount;
        Vec2i m_PlaneSizeShift[2];
        std::size_t m_PlaneDataOffset[2];
        std::size_t m_PlaneRowBytes[2];
    };

} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_META_PIXEL_SAMPLE_BUFFER_H
