#include "UQuadDevice.h"


namespace uquad
{
namespace hal
{
    #include <uQuad.X/uquad_device_types.h>
    
    UQuadDevice::UQuadDevice()
        : m_Acceleration()
        , m_RotationRate()
        , m_MagneticField()
        , m_AtmTemperature(0.0f)
        , m_AtmPressure(0.0f)
        , m_USBDevice()
    {
    }
    
    UQuadDevice::~UQuadDevice()
    {
    }
    
    bool UQuadDevice::isOpen() const
    {
        return m_USBDevice.isOpen();
    }
    
    system::error_code UQuadDevice::open()
    {
        return m_USBDevice.open(UQUAD_DEVICE_VENDOR_ID, UQUAD_DEVICE_PRODUCT_ID);
    }
    
    void UQuadDevice::close()
    {
    	m_USBDevice.close();
    }
    
    system::error_code UQuadDevice::start()
    {
    	if(system::error_code srce = m_USBDevice.setReceiveCallback(
    		bind(
				&UQuadDevice::packetReceived,
				this,
				_1,
				_2,
				_3
			)
    	)) return srce;

        if(system::error_code se = m_USBDevice.start()) return se;
        
        m_StartStopSignal(isStarted());
        return system::errc::make_error_code(system::errc::success);
    }
    
    bool UQuadDevice::isStarted()
    {
        return m_USBDevice.isOpen() && m_USBDevice.isStarted();
    }
    
    system::error_code UQuadDevice::stop()
    {
        if(!isStarted())
            return asio::error::make_error_code(asio::error::not_connected);;
        
        if(system::error_code se = m_USBDevice.stop()) return se;
        m_StartStopSignal(isStarted());
        return system::errc::make_error_code(system::errc::success);
    }
    
    void UQuadDevice::setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3)
    {
        if(!isStarted())
            return;
        uquad_device_packet_t packet;
        packet.packet.id = UQUAD_DEVICE_PKT_LEDS_MODE;
        packet.packet.leds_mode.led0_mode = lm0;
        packet.packet.leds_mode.led1_mode = lm1;
        packet.packet.leds_mode.led2_mode = lm2;
        packet.packet.leds_mode.led3_mode = lm3;
        m_USBDevice.transmitReport(packet.raw, sizeof(packet.raw));
    }
    
    void UQuadDevice::setMotorsPWM(float m0, float m1, float m2, float m3)
    {
        if(!isStarted())
            return;
        uquad_device_packet_t packet;
        packet.packet.id = UQUAD_DEVICE_PKT_MOTORS_PWM;
        packet.packet.motors_pwm.motor0_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m0)));
        packet.packet.motors_pwm.motor1_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m1)));
        packet.packet.motors_pwm.motor2_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m2)));
        packet.packet.motors_pwm.motor3_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m3)));
        m_USBDevice.transmitReport(packet.raw, sizeof(packet.raw));
    }
    
    
    void UQuadDevice::packetReceived(system::error_code const &e, uint8_t const *d, std::size_t s)
    {
    	if(e)
    	{
    		int c=0;
    	}
    	else
    	{
			uquad_device_packet_t const *pPacket = reinterpret_cast<uquad_device_packet_t const *>(d);
			switch(pPacket->packet.id)
			{
			case UQUAD_DEVICE_PKT_MEASUREMENT:

				m_Acceleration = Vec3f(
					pPacket->packet.measurement.acceleration.x/8192.0f,
					pPacket->packet.measurement.acceleration.y/8192.0f,
					pPacket->packet.measurement.acceleration.z/8192.0f
				);

				m_RotationRate = Vec3f(
					pPacket->packet.measurement.rotation_rate.x/65.536f,
					pPacket->packet.measurement.rotation_rate.y/65.536f,
					pPacket->packet.measurement.rotation_rate.z/65.536f
				);

				m_MagneticField = Vec3f(
					pPacket->packet.measurement.magnetic_field.x/6.0f,
					pPacket->packet.measurement.magnetic_field.y/6.0f,
					pPacket->packet.measurement.magnetic_field.z/6.0f
				);

				m_AtmTemperature = pPacket->packet.measurement.atm_temperature/256.0f;
				m_AtmPressure = (pPacket->packet.measurement.atm_pressure >> 4)/4.0f;
				m_DataReadySignal();
				break;
			}
    	}
    }
    
} //namespace hal
} //namespace uquad
