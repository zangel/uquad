#ifndef UQUAD_HAL_IOS_UQUAD_H
#define UQUAD_HAL_IOS_UQUAD_H

#include "../Config.h"
#include "../UQuad.h"

#include <AVFoundation/AVFoundation.h>
#include <CoreMotion/CoreMotion.h>

namespace uquad
{
namespace hal
{
namespace ios
{
    class UQuad
    	: public hal::UQuad
    {
    public:
        UQuad();
        ~UQuad();

        std::string const& name() const;

        bool isOpen() const;
        system::error_code open();
        system::error_code close();

        system::error_code getSensorsData(UQuadSensorsData &sd) const;

        system::error_code setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3);

        system::error_code armMotors();
        bool areMotorsRunning() const;
        system::error_code disarmMotors();

        system::error_code setMotorsPower(float mpfl, float mpfr, float mprl, float mprr);

    private:
        void handleCMSensors();

    private:
        uint8_t m_CMSensorsReadyFlags;
        UQuadSensorsData m_CMSensorsData;
        NSOperationQueue *m_OperationQueue;
        CMMotionManager *m_MotionManager;
        CMAltimeter *m_Altimeter;
        mutable fast_mutex m_SensorsDataGuard;
        UQuadSensorsData m_SensorsData;
    };

} //namespace ios
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_IOS_UQUAD_H
