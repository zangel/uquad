#include "GRAlloc.h"
#include "../Error.h"
#include <errno.h>

namespace uquad
{
namespace hal
{
namespace android
{
    GRAlloc::GRAlloc()
        : m_GRAllocModule(0)
        , m_GRAllocDevice(0)
    {
        hw_module_t const *hw_module = 0;
        if(::hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &hw_module) == 0)
        {
            if(::gralloc_open(hw_module, &m_GRAllocDevice) == 0)
            {
                m_GRAllocModule = reinterpret_cast<gralloc_module_t const *>(hw_module);
            }
        }
    }

    GRAlloc::~GRAlloc()
    {
        ::gralloc_close(m_GRAllocDevice);
        m_GRAllocDevice = 0;
        m_GRAllocModule = 0;
    }

    system::error_code GRAlloc::alloc(Vec2i const &size, int format, int usage, buffer_handle_t* handle, int32_t* stride)
    {
        if(format <= 0)
            return makeErrorCode(kEInvalidArgument);

        if(!size(0) || !size(1))
            return makeErrorCode(kEInvalidArgument);

        if(m_GRAllocDevice->alloc(m_GRAllocDevice, size(0), size(1), format, usage, handle, stride))
            return makeErrorCode(kEInvalidArgument);

        return makeErrorCode(kENoError);
    }

    system::error_code GRAlloc::free(buffer_handle_t handle)
    {
        if(m_GRAllocDevice->free(m_GRAllocDevice, handle))
            return makeErrorCode(kEInvalidArgument);

        return makeErrorCode(kENoError);
    }

    system::error_code GRAlloc::lock(buffer_handle_t handle, int usage, Vec2i const &size, void** vaddr)
    {
        if(m_GRAllocModule->lock(m_GRAllocModule, handle, usage, 0, 0, size(0), size(1), vaddr))
            return makeErrorCode(kEInvalidArgument);

        return makeErrorCode(kENoError);
    }

    system::error_code GRAlloc::lockYCbCr(buffer_handle_t handle, int usage, Vec2i const &size, android_ycbcr *ycbcr)
    {
        if(m_GRAllocModule->lock_ycbcr(m_GRAllocModule, handle, usage, 0, 0, size(0), size(1), ycbcr))
            return makeErrorCode(kEInvalidArgument);

        return makeErrorCode(kENoError);
    }

    system::error_code GRAlloc::unlock(buffer_handle_t handle)
    {
        if(m_GRAllocModule->unlock(m_GRAllocModule, handle))
            return makeErrorCode(kEInvalidArgument);

        return makeErrorCode(kENoError);
    }

    bool GRAlloc::shouldUseLockYCbCr(base::ePixelFormat format)
    {
        return format == base::kPixelFormatYUV420sp;
    }

    int GRAlloc::halPixelFormatFromPixelFormat(base::ePixelFormat format)
    {
        //HAL_PIXEL_FORMAT_NV12_ENCODEABLE
        switch(format)
        {
        case base::kPixelFormatRGB8:                return HAL_PIXEL_FORMAT_RGB_888;        break;
        case base::kPixelFormatBGRA8:               return HAL_PIXEL_FORMAT_BGRA_8888;      break;
        case base::kPixelFormatYUV420sp:            return HAL_PIXEL_FORMAT_YCbCr_420_888;  break;
        default: break;
        }
        return -1;
    }

    base::ePixelFormat GRAlloc::pixelFormatFromHalPixelFormat(int format)
    {
        switch(format)
        {
        case HAL_PIXEL_FORMAT_RGB_888:              return base::kPixelFormatRGB8;          break;
        case HAL_PIXEL_FORMAT_BGRA_8888:            return base::kPixelFormatBGRA8;         break;
        case HAL_PIXEL_FORMAT_YCbCr_420_888:         return base::kPixelFormatYUV420sp;     break;
        default: break;
        }
        return base::kPixelFormatInvalid;
    }

} //namespace android
} //namespace hal
} //namespace uquad
