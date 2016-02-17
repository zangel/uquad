#include "DeviceManager.h"

namespace uquad
{
namespace hal
{
namespace usb
{
	DeviceManager::DeviceManager()
        : m_bStarted(false)
		, m_bStop(false)
		, m_StartStopMutex()
		, m_WorkerThread()
		, m_USBContext(0)
    {
    }

	DeviceManager::~DeviceManager()
    {
        stop();
    }

    bool DeviceManager::isStarted() const
	{
		return m_bStarted;
	}

    system::error_code DeviceManager::start()
	{
    	mutex::scoped_lock lock(m_StartStopMutex);
    	if(isStarted())
			return asio::error::make_error_code(asio::error::already_started);

    	if(libusb_init(&m_USBContext) || !m_USBContext)
    	{
    		return asio::error::make_error_code(asio::error::not_found);
    	}

    	//libusb_set_debug(m_USBContext, LIBUSB_LOG_LEVEL_INFO);


    	m_bStop = false;
    	m_WorkerThread = thread(bind(&DeviceManager::workerThread, this));
    	while(!m_bStarted) this_thread::sleep(posix_time::milliseconds(10));

    	return system::errc::make_error_code(system::errc::success);
	}

    system::error_code DeviceManager::stop()
	{
    	mutex::scoped_lock lock(m_StartStopMutex);
    	if(!isStarted())
			return asio::error::make_error_code(asio::error::not_found);

    	m_bStop = true;
    	while(m_bStarted) this_thread::sleep(posix_time::milliseconds(10));

    	m_WorkerThread.join();

    	libusb_exit(m_USBContext);
		m_USBContext = 0;
		return system::errc::make_error_code(system::errc::success);
	}

    system::error_code DeviceManager::enumerateDevices(uint16_t vid, uint16_t pid, std::vector<DeviceInfo> &devices)
	{
		if(!isStarted())
			return asio::error::make_error_code(asio::error::not_found);

		devices.clear();

		libusb_device **devs;
		std::size_t num_devs = libusb_get_device_list(m_USBContext, &devs);

		if(!num_devs)
			return asio::error::make_error_code(asio::error::not_found);

		for(std::size_t d = 0; d < num_devs; ++d)
		{
			libusb_device *dev = devs[d];
			if(!dev)
				continue;

			libusb_device_descriptor desc;
			if(libusb_get_device_descriptor(dev, &desc) < 0)
				continue;

			if(vid != desc.idVendor || pid != desc.idProduct)
				continue;

			libusb_config_descriptor *conf_desc = 0;
			if(libusb_get_active_config_descriptor(dev, &conf_desc) < 0)
				libusb_get_config_descriptor(dev, 0, &conf_desc);

			if(conf_desc)
			{
				for(int i = 0; i < conf_desc->bNumInterfaces; i++)
				{
					libusb_interface const *intf = &conf_desc->interface[i];
					for(int as = 0; as < intf->num_altsetting; as++)
					{
						libusb_interface_descriptor const *intf_desc = &intf->altsetting[as];
						if(intf_desc->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC)
						{
							DeviceInfo dev_info;

							dev_info.device = dev;
							dev_info.interface = intf_desc->bInterfaceNumber;
							dev_info.inputEP = 0;
							dev_info.inputEPPacketSize = 0;
							dev_info.outputEP = 0;
							dev_info.outputEPPacketSize = 0;



							for(int iep = 0; iep < intf_desc->bNumEndpoints; iep++)
							{
								libusb_endpoint_descriptor const *ep = &intf_desc->endpoint[iep];
								bool is_bulk = (ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK;
								bool is_output = (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT;
								bool is_input = (ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN;

								if(dev_info.inputEP == 0 && is_bulk && is_input)
								{
									dev_info.inputEP = ep->bEndpointAddress;
									dev_info.inputEPPacketSize = ep->wMaxPacketSize;
								}

								if(dev_info.outputEP == 0 && is_bulk && is_output)
								{
									dev_info.outputEP = ep->bEndpointAddress;
									dev_info.outputEPPacketSize = ep->wMaxPacketSize;
								}
							}

							if(	dev_info.inputEP > 0 && dev_info.inputEPPacketSize > 0 &&
								dev_info.outputEP > 0 && dev_info.outputEPPacketSize > 0)
							{
								devices.push_back(dev_info);
							}
						}
					}
				}
				libusb_free_config_descriptor(conf_desc);
			}
		}
		libusb_free_device_list(devs, 1);
		return system::errc::make_error_code(system::errc::success);
	}

    void DeviceManager::workerThread()
	{
    	m_bStarted = true;
    	timeval tout;
    	tout.tv_sec = 0;
    	tout.tv_usec = 500000;
    	while(!m_bStop)
		{
    		libusb_handle_events_timeout(m_USBContext, &tout);
		}
    	m_bStarted = false;
	}

} //namespace usb
} //namespace hal
} //namespace uquad
