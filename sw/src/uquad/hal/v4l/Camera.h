#ifndef UQUAD_HAL_V4L_CAMERA_H
#define UQUAD_HAL_V4L_CAMERA_H

#include "../Camera.h"

namespace uquad
{
namespace hal
{
    class Camera::Impl
    {
    public:
    	static uint32_t numCameras();

    	Impl(Camera &camera, uint32_t camidx);
    	~Impl();

        bool isOpen() const;
        system::error_code open();
        system::error_code close();

        system::error_code getCaptureFPSRange(FPSRange &fr) const;

    private:
        Camera &m_Camera;
        int m_FD;
    };

} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_V4L_CAMERA_H
