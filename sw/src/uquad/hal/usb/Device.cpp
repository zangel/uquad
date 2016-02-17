#include "Device.h"
#include "DeviceManager.h"

#define UQUAD_HAL_USB_DEVICE_RECEIVE_QUEUE_SIZE 3
#define UQUAD_HAL_USB_DEVICE_TRANSMIT_QUEUE_SIZE 3


namespace uquad
{
namespace hal
{
namespace usb
{
    Device::Device()
        : m_DeviceInfo()
    	, m_DeviceHandle(0)
    	, m_StartStopMutex()
    	, m_bStarted(false)
    	, m_bStop(false)
    	, m_ReceiveTransfersBuffer()
    	, m_TransmitTransfersBuffer()
    	, m_ReceiveTransfersPool(UQUAD_HAL_USB_DEVICE_RECEIVE_QUEUE_SIZE)
        , m_TransmitTransfersPool(UQUAD_HAL_USB_DEVICE_TRANSMIT_QUEUE_SIZE)
    	, m_TransmitTransfers(UQUAD_HAL_USB_DEVICE_TRANSMIT_QUEUE_SIZE)
    	, m_CurrentTransmitTransfer(0)
    	, m_ReceiveCallbackFn()
    {
    }

    Device::~Device()
    {
        close();
    }

    bool Device::isOpen() const
    {
        return m_DeviceHandle != 0;
    }

    system::error_code Device::open(uint16_t vendor, uint16_t product)
	{
		if(isOpen())
			return asio::error::already_open;

		if(!utility::Singleton<DeviceManager>::instance().isStarted() && utility::Singleton<DeviceManager>::instance().start())
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

		std::vector<DeviceInfo> matching_devices;

		system::error_code err_ed = utility::Singleton<DeviceManager>::instance().enumerateDevices(vendor, product, matching_devices);

		if(err_ed)
			return err_ed;

		if(matching_devices.empty())
			return asio::error::not_found;

		libusb_device_handle *dev_found = 0;
		std::size_t idi = 0;
		for(idi = 0; (idi < matching_devices.size() && !dev_found); ++idi)
		{
			DeviceInfo const &dev_info = matching_devices[idi];

			libusb_device_handle *handle = 0;
			if(libusb_open(dev_info.device, &handle) < 0 || !handle)
				continue;

			if(libusb_kernel_driver_active(handle, dev_info.interface) == 1)
			{
				if(libusb_detach_kernel_driver(handle, dev_info.interface) < 0)
				{
					libusb_close(handle);
					continue;
				}
			}

			if(libusb_claim_interface(handle, dev_info.interface) < 0)
			{
				libusb_close(handle);
				continue;
			}

			dev_found = handle;
			break;
		}

		if(!dev_found)
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

		m_DeviceInfo =  matching_devices[idi];
		m_DeviceHandle = dev_found;
		m_ReceiveTransfersBuffer.resize(UQUAD_HAL_USB_DEVICE_RECEIVE_QUEUE_SIZE*m_DeviceInfo.inputEPPacketSize);
		m_TransmitTransfersBuffer.resize(UQUAD_HAL_USB_DEVICE_TRANSMIT_QUEUE_SIZE*m_DeviceInfo.outputEPPacketSize);

		for(int irt = 0; irt < UQUAD_HAL_USB_DEVICE_RECEIVE_QUEUE_SIZE; ++irt)
		{
			libusb_transfer *rt = libusb_alloc_transfer(0);
			libusb_fill_bulk_transfer(
				rt,
				m_DeviceHandle,
				m_DeviceInfo.inputEP,
				m_ReceiveTransfersBuffer.data() + irt*m_DeviceInfo.inputEPPacketSize,
				m_DeviceInfo.inputEPPacketSize,
				&Device::_receiveCallback,
				this,
				10
			);
			m_ReceiveTransfersPool.push(rt);
		}

		for(int itt = 0; itt < UQUAD_HAL_USB_DEVICE_TRANSMIT_QUEUE_SIZE; ++itt)
		{
			libusb_transfer *tt = libusb_alloc_transfer(0);
			libusb_fill_bulk_transfer(
				tt,
				m_DeviceHandle,
				m_DeviceInfo.outputEP,
				m_TransmitTransfersBuffer.data() + itt*m_DeviceInfo.outputEPPacketSize,
				m_DeviceInfo.outputEPPacketSize,
				&Device::_transmitCallback,
				this,
				10
			);
			m_TransmitTransfersPool.push(tt);
		}
		m_TransmitTransfers.reset();

		return system::errc::make_error_code(system::errc::success);
	}

    void Device::close()
    {
    	if(!isOpen() || !utility::Singleton<DeviceManager>::instance().isStarted())
    		return;

    	libusb_transfer *t;
    	while(m_ReceiveTransfersPool.pop(t))
    		libusb_free_transfer(t);

    	while(m_TransmitTransfersPool.pop(t))
			libusb_free_transfer(t);

    	m_TransmitTransfers.reset();


    	m_ReceiveTransfersBuffer.clear();
    	m_TransmitTransfersBuffer.clear();

    	libusb_close(m_DeviceHandle);
    	m_DeviceHandle = 0;
    }

    system::error_code Device::setReceiveCallback(Device::ReceiveCallbackFn rcb)
    {
    	if(isStarted())
    		return system::errc::make_error_code(system::errc::not_supported);

    	m_ReceiveCallbackFn = rcb;
    	return system::errc::make_error_code(system::errc::success);
    }

    bool Device::isStarted() const
    {
    	return m_bStarted;
    }

    system::error_code Device::start()
    {
    	mutex::scoped_lock  lock(m_StartStopMutex);

    	if(!isOpen() || !utility::Singleton<DeviceManager>::instance().isStarted())
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

		if(isStarted())
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

		m_bStop = false;
		m_TransmitTransfers.reset();
		m_CurrentTransmitTransfer.store(0);

		libusb_transfer *t;
		if(!m_ReceiveTransfersPool.pop(t))
		{
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
		}

		if(libusb_submit_transfer(t) < 0)
		{
			return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
		}

		m_bStarted = true;
		return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Device::stop()
	{
    	mutex::scoped_lock lock(m_StartStopMutex);

		if(!isStarted())
			return system::errc::make_error_code(system::errc::already_connected);

		m_bStop = true;

		while(m_ReceiveTransfersPool.write_available())
			this_thread::sleep(posix_time::milliseconds(10));

		while(m_TransmitTransfersPool.write_available())
			this_thread::sleep(posix_time::milliseconds(10));

		m_TransmitTransfers.reset();
		m_bStarted = false;

		return system::errc::make_error_code(system::errc::success);
	}

    system::error_code Device::transmitReport(uint8_t const *data, uint32_t len)
	{
    	if(!isStarted() || m_bStop)
    		return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

    	libusb_transfer *t;
    	if(!m_TransmitTransfersPool.pop(t))
    	{
    		return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    	}
    	memcpy(t->buffer, data, len);
    	m_TransmitTransfers.push(t);
    	return startTransmit();
	}

    void Device::_receiveCallback(libusb_transfer *transfer)
    {
    	Device *device = reinterpret_cast<Device*>(transfer->user_data);
		if(!device)
			return;
		device->receiveCallback(transfer);
    }

    void Device::_transmitCallback(libusb_transfer *transfer)
    {
    	Device *device = reinterpret_cast<Device*>(transfer->user_data);
		if(!device)
			return;
		device->transmitCallback(transfer);
    }

    void Device::receiveCallback(libusb_transfer *transfer)
    {
    	m_ReceiveTransfersPool.push(transfer);

    	switch(transfer->status)
    	{
    	case LIBUSB_TRANSFER_COMPLETED:
    		{
    			static system::error_code ec = system::errc::make_error_code(system::errc::success);
    			if(m_ReceiveCallbackFn.is_initialized())
    				m_ReceiveCallbackFn.get()(ec, transfer->buffer, transfer->actual_length);
    		}
    		break;

    	case LIBUSB_TRANSFER_ERROR:
    		{
    			static system::error_code ec = system::errc::make_error_code(system::errc::broken_pipe);
				if(m_ReceiveCallbackFn.is_initialized())
					m_ReceiveCallbackFn.get()(ec, 0, 0);
    		}
    		break;

    	case LIBUSB_TRANSFER_TIMED_OUT:
			{
				static system::error_code ec = system::errc::make_error_code(system::errc::timed_out);
				if(m_ReceiveCallbackFn.is_initialized())
					m_ReceiveCallbackFn.get()(ec, 0, 0);
			}
			break;


    	case LIBUSB_TRANSFER_CANCELLED:
    		{
    			static system::error_code ec = system::errc::make_error_code(system::errc::connection_aborted);
    			if(m_ReceiveCallbackFn.is_initialized())
    				m_ReceiveCallbackFn.get()(ec, 0, 0);
    		}
    		break;

    	case LIBUSB_TRANSFER_STALL:
			{
				static system::error_code ec = system::errc::make_error_code(system::errc::timed_out);
				if(m_ReceiveCallbackFn.is_initialized())
					m_ReceiveCallbackFn.get()(ec, 0, 0);
			}
			break;

    	case LIBUSB_TRANSFER_NO_DEVICE:
    		{
    			static system::error_code ec = system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    			if(m_ReceiveCallbackFn.is_initialized())
    				m_ReceiveCallbackFn.get()(ec, 0, 0);
    		}
    		break;

    	case LIBUSB_TRANSFER_OVERFLOW:
    		{
				static system::error_code ec = system::errc::make_error_code(system::errc::timed_out);
				if(m_ReceiveCallbackFn.is_initialized())
					m_ReceiveCallbackFn.get()(ec, 0, 0);
			}
			break;
    	}

    	if(isStarted() && !m_bStop)
		{
			libusb_transfer *t;
			if(m_ReceiveTransfersPool.pop(t))
			{
				if(libusb_submit_transfer(t) < 0)
				{
					m_ReceiveTransfersPool.push(t);
				}
			}
		}
    }

    void Device::transmitCallback(libusb_transfer *transfer)
    {
    	switch(transfer->status)
		{
		case LIBUSB_TRANSFER_COMPLETED:
			{

			}
			break;

		case LIBUSB_TRANSFER_ERROR:
			{

			}
			break;

		case LIBUSB_TRANSFER_TIMED_OUT:
			{

			}
			break;


		case LIBUSB_TRANSFER_CANCELLED:
			{

			}
			break;

		case LIBUSB_TRANSFER_STALL:
			{

			}
			break;

		case LIBUSB_TRANSFER_NO_DEVICE:
			{

			}
			break;

		case LIBUSB_TRANSFER_OVERFLOW:
			{

			}
			break;
		}



		m_TransmitTransfersPool.push(transfer);
		m_TransmitTransfers.pop();

		m_CurrentTransmitTransfer.store(0);
		startTransmit();

    }

    system::error_code Device::startTransmit()
    {
    	if(!m_TransmitTransfers.read_available())
    		return system::errc::make_error_code(system::errc::resource_unavailable_try_again);

    	libusb_transfer *eold = 0;

    	if(m_CurrentTransmitTransfer.compare_exchange_strong(eold, m_TransmitTransfers.front()))
    	{
    		while(m_TransmitTransfers.read_available())
    		{
    			if(libusb_submit_transfer(m_TransmitTransfers.front()) < 0)
    			{
    				libusb_transfer *t;
    				m_TransmitTransfers.pop(t);
    				m_TransmitTransfersPool.push(t);
    				if(!m_TransmitTransfers.read_available())
    				{
    					return system::errc::make_error_code(system::errc::resource_unavailable_try_again);
    				}
    			}
    			else
    			{
    				break;
    			}
    		}
    	}
    	return system::errc::make_error_code(system::errc::success);
    }


} //namespace usb
} //namespace hal
} //namespace uquad
