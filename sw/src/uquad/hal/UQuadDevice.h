#ifndef UQUAD_HAL_UQUAD_DEVICE_H
#define UQUAD_HAL_UQUAD_DEVICE_H

#include "Config.h"
#include "Device.h"
#include "usb/Device.h"

namespace uquad
{
namespace hal
{
    class UQuadDevice
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
            kLedModeOn,
        } eLedMode;
        
        typedef uquad::signal< void () > DataReadySignal;
        typedef uquad::signal< void (bool) > StartStopSignal;
        
        UQuadDevice();
        ~UQuadDevice();
        
        bool isOpen() const;
        system::error_code open();
        system::error_code close();
        
        system::error_code start();
        bool isStarted();
        system::error_code stop();
        
        void setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3);
        void setMotorsPWM(float m0, float m1, float m2, float m3);
        
        
    protected:
        void packetReceived(system::error_code const &e, uint8_t const *d, std::size_t s);
        
    public:
        Vec3f m_Acceleration;
        Vec3f m_RotationRate;
        Vec3f m_MagneticField;
        float m_AtmTemperature;
        float m_AtmPressure;
        
        DataReadySignal m_DataReadySignal;
        StartStopSignal m_StartStopSignal;
        
    private:
        usb::Device m_USBDevice;
    };
    
} //namespace hal
} //namespace uquad

#endif //UQUAD_HAL_UQUAD_DEVICE_H
