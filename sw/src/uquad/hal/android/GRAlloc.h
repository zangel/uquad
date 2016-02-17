#ifndef UQUAD_HAL_ANDROID_GRALLOC_H
#define UQUAD_HAL_ANDROID_GRALLOC_H

#include "../Config.h"
#include <system/graphics.h>
#include <hardware/gralloc.h>
#include "../../base/PixelFormat.h"

namespace uquad
{
namespace hal
{
namespace android
{
    class GRAlloc
    {
    public:
    protected:
        GRAlloc();

    public:
        inline bool isOpened() const { return m_GRAllocDevice != 0; }

    public:
       ~GRAlloc();

       system::error_code alloc(Vec2i const &size, int format, int usage, buffer_handle_t* handle, int32_t* stride);
       system::error_code free(buffer_handle_t handle);

       system::error_code lock(buffer_handle_t handle, int usage, Vec2i const &size, void** vaddr);
       system::error_code lockYCbCr(buffer_handle_t handle, int usage, Vec2i const &size, android_ycbcr *ycbcr);
       system::error_code unlock(buffer_handle_t handle);

       static bool shouldUseLockYCbCr(base::ePixelFormat format);
       static int halPixelFormatFromPixelFormat(base::ePixelFormat format);
       static base::ePixelFormat pixelFormatFromHalPixelFormat(int format);


    private:
        gralloc_module_t const *m_GRAllocModule;
        alloc_device_t *m_GRAllocDevice;
    };

} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_GRALLOC_H
