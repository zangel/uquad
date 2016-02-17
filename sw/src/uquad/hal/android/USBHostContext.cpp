#include "USBHostContext.h"
#include "../Log.h"

namespace uquad
{
namespace hal
{
namespace android
{
    namespace
    {
        struct OpenDeviceOp
        {
            OpenDeviceOp(uint16_t v, uint16_t p)
                : vendor(v)
                , product(p)
                , device(0)
            {
            }

            static int deviceAdded(const char *name, void *cd)
            {
                OpenDeviceOp *op = reinterpret_cast<OpenDeviceOp*>(cd);

                if(!op)
                {
                    UQUAD_HAL_LOG(Error) << "USBHostContext::OpenDeviceOp::deviceAdded: passed null op!";
                    return 0;
                }

                usb_device *dev = usb_device_open(name);
                if(!dev)
                {
                    UQUAD_HAL_LOG(Error) << "USBHostContext::OpenDeviceOp::deviceAdded: failed to open device: " << name;
                    return 0;
                }

                if(op->vendor != usb_device_get_vendor_id(dev) || op->product != usb_device_get_product_id(dev))
                {
                    usb_device_close(dev);
                    return 0;
                }

                op->device = dev;
                UQUAD_HAL_LOG(Debug) << "USBHostContext::OpenDeviceOp::deviceAdded: device opened: " << name;
                return 1;
            }

            static int discoveryDone(void *cd)
            {
                UQUAD_HAL_LOG(Debug) << "USBHostContext::OpenDeviceOp::discoveryDone!";
                return 1;
            }

            uint16_t vendor;
            uint16_t product;
            usb_device *device;
        };
    }

    USBHostContext::USBHostContext()
        : m_Context(usb_host_init())
    {
        if(m_Context)
            UQUAD_HAL_LOG(Debug) << "USBHostContext::USBHostContext: context acquired!";
        else
            UQUAD_HAL_LOG(Error) << "USBHostContext::USBHostContext: failed to acquire context!";
    }

    USBHostContext::~USBHostContext()
    {
        if(m_Context)
        {
            usb_host_cleanup(m_Context);
            m_Context = 0;
            UQUAD_HAL_LOG(Debug) << "USBHostContext::~USBHostContext: context released!";
        }
    }

    usb_device* USBHostContext::openDevice(uint16_t vendor, uint16_t product)
    {
        OpenDeviceOp op(vendor, product);
        usb_host_run(m_Context,
                &OpenDeviceOp::deviceAdded,
                0,
                &OpenDeviceOp::discoveryDone,
                &op
        );

        return op.device;
    }

} //namespace android
} //namespace hal
} //namespace uquad
