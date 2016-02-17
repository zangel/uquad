#ifndef UQUAD_HAL_UQUAD_H
#define UQUAD_HAL_UQUAD_H

#include "Config.h"
#include "Device.h"
#include "../base/Time.h"
#include "../base/Serialization.h"


namespace uquad
{
namespace hal
{
    class UQuadSensorsData
    {
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
        
        base::TimePoint time;
        Vec3f velocityRate;
        Vec3f rotationRate;
        Vec3f magneticField;
        float baroTemperature;
        float baroPressure;
    };
    
    class UQuadDelegate
	{
	public:
	    UQuadDelegate() = default;
		virtual ~UQuadDelegate() = default;
        virtual void onUQuadSensorsReady() {}
	};


    class UQuad
    	: public Device
    {
    public:
        typedef enum
        {
            kLedMode0,
            kLedMode1,
            kLedMode2,
            kLedMode3,
            kLedMode4,
            kLedMode5,
            kLedMode6,
            kLedMode7,
            kLedModeBoot0,
            kLedModeBoot1,
            kLedModeBoot2,
            kLedModeBoot3,
            kLedModeOn,
            kLedModeCount
        } eLedMode;
        
        typedef base::Delegate<UQuadDelegate> uquad_delegate_t;

    	UQuad();
        ~UQuad();

        Device::Type type() const;

        virtual system::error_code getSensorsData(UQuadSensorsData &sd) const = 0;

        virtual system::error_code setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3) = 0;

        virtual system::error_code armMotors() = 0;
        virtual bool areMotorsRunning() const = 0;
        virtual system::error_code disarmMotors() = 0;
        virtual system::error_code setMotorsPower(float mpfl, float mpfr, float mprl, float mprr) = 0;


        inline uquad_delegate_t const& uquadDelegate() { return m_UQuadDelegate; }

    protected:

        inline void notifyOnUQuadSensorsReady()
        {
            m_UQuadDelegate.call(&UQuadDelegate::onUQuadSensorsReady);
        }

    private:
        uquad_delegate_t m_UQuadDelegate;
    };

} //namespace hal
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::hal::UQuadSensorsData)

#endif //UQUAD_HAL_CAMERA_H
