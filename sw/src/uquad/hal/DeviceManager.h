#ifndef UQUAD_HAL_DEVICE_MANAGER_H
#define UQUAD_HAL_DEVICE_MANAGER_H

#include "Config.h"
#include "Device.h"

namespace uquad
{
namespace hal
{
	class DeviceManager
	{
	public:
		typedef mi::multi_index_container
		<
			intrusive_ptr<Device>,
			mi::indexed_by
			<
				mi::random_access<>,
				mi::hashed_unique< mi::identity< intrusive_ptr<Device> > >,
				mi::ordered_non_unique
				<
					mi::const_mem_fun
					<
						Device,
						Device::Type,
						&Device::type
					>
				>,
				mi::hashed_unique
				<
					mi::const_mem_fun
					<
						Device,
						std::string const &,
						&Device::name
					>
				>
			>
		> devices_t;

		typedef mi::nth_index<devices_t, 1>::type devices_by_identity_t;
		typedef mi::nth_index<devices_t, 2>::type devices_by_type_t;
		typedef mi::nth_index<devices_t, 3>::type devices_by_name_t;


	protected:
		DeviceManager();

	public:
       ~DeviceManager();

       bool registerDevice(intrusive_ptr<Device> const &d);
       bool deregisterDevice(intrusive_ptr<Device> const &d);

       devices_t const& getDevices() const;
       intrusive_ptr<Device> findDevice(std::string const &name) const;

       template <typename D>
       intrusive_ptr<D> findDevice(std::string const &name) const
       {
    	   return dynamic_pointer_cast<D>(findDevice(name));
       }

       std::pair<
       	   devices_by_type_t::const_iterator,
       	   devices_by_type_t::const_iterator
       > findDevicesByType(Device::Type type) const;

       template <typename D, typename Exp>
       inline void enumerateDevices(Exp exp)
       {
    	   std::pair<
    	   	   devices_by_type_t::const_iterator,
    	       devices_by_type_t::const_iterator
    	   > devices = findDevicesByType(Device::Type(typeid(D)));

    	   for(devices_by_type_t::const_iterator id = devices.first; id != devices.second; ++id)
    	   {
    		   intrusive_ptr<D> d = dynamic_pointer_cast<D>(*id);
    		   if(!d) continue;
    		   if(!exp(d)) break;
    	   }
       }



	private:
       mutable mutex m_Sync;
       devices_t m_Devices;
	};

} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_DEVICE_MANAGER_H
