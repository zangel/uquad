#ifndef UQUAD_HAL_DEVICE_H
#define UQUAD_HAL_DEVICE_H

#include "Config.h"

#include "../base/RefCounted.h"
#include "../base/Delegate.h"

namespace uquad
{
namespace hal
{
	class DeviceManager;

	class DeviceDelegate
	{
	public:
		DeviceDelegate() = default;
		virtual ~DeviceDelegate() = default;

		virtual void onDeviceOpened() {}
		virtual void onDeviceClosed() {}
	};

    class Device
    	: public base::RefCounted
    {
    public:
        typedef std::type_index Type;
    	typedef base::Delegate<DeviceDelegate> device_delegate_t;

    	Device();
    	~Device();

    	virtual Type type() const = 0;
    	virtual std::string const& name() const = 0;

    	virtual bool isOpen() const = 0;
    	virtual system::error_code open() = 0;
    	virtual system::error_code close() = 0;

    	template <typename D>
    	static system::error_code registerDevices(DeviceManager &dm);

    	inline device_delegate_t const& deviceDelegate() { return m_DeviceDelegate; }
        
    protected:
        inline void notifyOnDeviceOpened()
        {
            m_DeviceDelegate.call(&DeviceDelegate::onDeviceOpened);
        }
        
        inline void notifyOnDeviceClosed()
        {
            m_DeviceDelegate.call(&DeviceDelegate::onDeviceClosed);
        }
        

    protected:
    	device_delegate_t m_DeviceDelegate;
    };

} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_UQUAD_DEVICE_H
