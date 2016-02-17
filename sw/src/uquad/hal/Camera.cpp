#include "Camera.h"

namespace uquad
{
namespace hal
{
	Camera::Camera()
	{

	}

	Camera::~Camera()
	{

	}

	Device::Type Camera::type() const
	{
		return typeid(Camera);
	}
    

} //namespace hal
} //namespace uquad
