#ifndef UQUAD_HAL_USB_DEVICE_INFO_H
#define UQUAD_HAL_USB_DEVICE_INFO_H

#include "../../Config.h"

namespace uquad
{
namespace hal
{
namespace usb
{
    struct DeviceInfo
    {
    public:
    	libusb_device *device;
    	int interface;
    	uint8_t inputEP;
    	uint16_t inputEPPacketSize;
    	uint8_t outputEP;
    	uint16_t outputEPPacketSize;
    };

} //namespace usb
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_USB_DEVICE_INFO_H

