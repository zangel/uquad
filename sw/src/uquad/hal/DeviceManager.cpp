#include "DeviceManager.h"
#include "Camera.h"
#include "UQuad.h"

namespace uquad
{
namespace hal
{
	DeviceManager::DeviceManager()
		: m_Sync()
		, m_Devices()
	{
		Device::registerDevices<Camera>(*this);
		Device::registerDevices<UQuad>(*this);
	}

	DeviceManager::~DeviceManager()
	{

	}

	bool DeviceManager::registerDevice(intrusive_ptr<Device> const &d)
	{
		lock_guard<mutex> lock(m_Sync);
		return m_Devices.push_back(d).second;
	}

	bool DeviceManager::deregisterDevice(intrusive_ptr<Device> const &d)
	{
		lock_guard<mutex> lock(m_Sync);
		devices_by_identity_t &devices_by_identity = m_Devices.get<1>();
		devices_by_identity_t::iterator itd = devices_by_identity.find(d);
		if(itd == devices_by_identity.end())
		{
			return false;
		}
		devices_by_identity.erase(itd);
		return true;
	}

	DeviceManager::devices_t const& DeviceManager::getDevices() const
	{
		return m_Devices;
	}

	intrusive_ptr<Device> DeviceManager::findDevice(std::string const &name) const
	{
		devices_by_name_t const &devices_by_name = m_Devices.get<3>();
		devices_by_name_t::const_iterator itd = devices_by_name.find(name);
		if(itd == devices_by_name.end())
		{
			return intrusive_ptr<Device>();
		}

		return *itd;
	}

	std::pair<
		DeviceManager::devices_by_type_t::const_iterator,
		DeviceManager::devices_by_type_t::const_iterator
	> DeviceManager::findDevicesByType(Device::Type type) const
	{
		devices_by_type_t const &devices_by_type = m_Devices.get<2>();
		return devices_by_type.equal_range(type);
	}


} //namespace hal
} //namespace uquad
