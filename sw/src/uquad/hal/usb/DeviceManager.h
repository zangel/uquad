#ifndef UQUAD_HAL_USB_DEVICE_MANAGER_H
#define UQUAD_HAL_USB_DEVICE_MANAGER_H

#include "../Config.h"
#include "DeviceInfo.h"

namespace uquad
{
namespace hal
{
namespace usb
{
    class DeviceManager
    {
    protected:
    	DeviceManager();

    public:
       ~DeviceManager();

        bool isStarted() const;
        system::error_code start();
        system::error_code stop();

        system::error_code enumerateDevices(uint16_t vid, uint16_t pid, std::vector< DeviceInfo > &devices);

    protected:
        void workerThread();
        void transmitThread();


    private:
        atomic<bool> m_bStarted;
        atomic<bool> m_bStop;
        mutex m_StartStopMutex;
        thread m_WorkerThread;

        libusb_context *m_USBContext;
    };

} //namespace usb
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_USB_DEVICE_MANAGER_H
