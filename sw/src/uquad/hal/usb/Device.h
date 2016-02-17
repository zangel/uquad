#ifndef UQUAD_HAL_USB_DEVICE_H
#define UQUAD_HAL_USB_DEVICE_H

#include "../Config.h"
#include "DeviceInfo.h"

namespace uquad
{
namespace hal
{
namespace usb
{
    class Device
    {
    public:
    	typedef function<void (system::error_code const &, uint8_t const *, std::size_t)> ReceiveCallbackFn;

        Device();
        ~Device();

        bool isOpen() const;
        system::error_code open(uint16_t vendor, uint16_t product);
        void close();

        system::error_code setReceiveCallback(ReceiveCallbackFn rcb);


        bool isStarted() const;
        system::error_code start();
        system::error_code stop();

        system::error_code transmitReport(uint8_t const *data, uint32_t len);

    private:
        static void _receiveCallback(libusb_transfer *transfer);
        static void _transmitCallback(libusb_transfer *transfer);

        void receiveCallback(libusb_transfer *transfer);
        void transmitCallback(libusb_transfer *transfer);

        system::error_code startTransmit();

    private:
        DeviceInfo m_DeviceInfo;
        libusb_device_handle *m_DeviceHandle;

        mutex m_StartStopMutex;
        atomic<bool> m_bStarted;
        atomic<bool> m_bStop;


        std::vector<uint8_t> m_ReceiveTransfersBuffer;
        std::vector<uint8_t> m_TransmitTransfersBuffer;

        lockfree::spsc_queue<libusb_transfer*> m_ReceiveTransfersPool;
        lockfree::spsc_queue<libusb_transfer*> m_TransmitTransfersPool;
        lockfree::spsc_queue<libusb_transfer*> m_TransmitTransfers;
        atomic<libusb_transfer*> m_CurrentTransmitTransfer;

        optional<ReceiveCallbackFn> m_ReceiveCallbackFn;
    };

} //namespace usb
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_USB_DEVICE_H
