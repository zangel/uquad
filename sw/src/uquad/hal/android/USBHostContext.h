#ifndef UQUAD_HAL_ANDROID_USB_HOST_CONTEXT_H
#define UQUAD_HAL_ANDROID_USB_HOST_CONTEXT_H

#include "../Config.h"
#include <usbhost/usbhost.h>
#include <linux/usbdevice_fs.h>

namespace uquad
{
namespace hal
{
namespace android
{
    class USBHostContext
    {
    public:
        struct IOCTLReapUrbNoDelay
        {
            IOCTLReapUrbNoDelay() : urb(0) { }

            inline int name() const { return USBDEVFS_REAPURBNDELAY; }

            inline void* data() { return &urb; }

            inline usb_request* request() { return urb?reinterpret_cast<usb_request*>(urb->usercontext):0; }
            usbdevfs_urb *urb;
        };

    protected:
        USBHostContext();

    public:
        inline bool isValid() const { return m_Context != 0; }

        inline usb_host_context* context() { return m_Context; }

        usb_device* openDevice(uint16_t vendor, uint16_t product);


    public:
       ~USBHostContext();

    private:
    private:
       usb_host_context *m_Context;
    };


} //namespace android
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_ANDROID_GRALLOC_H
