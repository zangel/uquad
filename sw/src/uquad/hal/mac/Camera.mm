#include "Camera.h"
#include "../DeviceManager.h"
#include "../av/Camera.h"

namespace uquad
{
namespace hal
{
	template<>
	system::error_code Device::registerDevices<Camera>(DeviceManager &dm)
	{
		return av::Camera::registerCameras(dm);
	}

} //namespace hal
} //namespace uquad
