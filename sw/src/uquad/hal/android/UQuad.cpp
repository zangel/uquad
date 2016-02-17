#include "UQuad.h"
#include "../DeviceManager.h"
#include "../Log.h"
#include "../../base/Error.h"
#include "USBHostContext.h"
#include "IOService.h"



namespace uquad
{
namespace hal
{

    template<>
    system::error_code Device::registerDevices<UQuad>(DeviceManager &dm)
    {
        if( !SI(android::IOService).isStarted() || !SI(android::USBHostContext).isValid())
        {
            return base::makeErrorCode(base::kENoSuchDevice);
        }

        dm.registerDevice(
            intrusive_ptr<UQuad>(new android::UQuad())
        );

        return base::makeErrorCode(base::kENoError);
    }

namespace android
{
    UQuad::MotorsStateMachine::MotorsStateMachine(UQuad &uq)
        : uquad(uq)
        , timer(SI(IOService).ios())
    {
    }

    template <typename Event>
    void UQuad::MotorsStateMachine::dispatchEvent(Event e)
    {
        SI(IOService).ios().dispatch([this, e]()
        {
            try
            {
                uquad.m_MotorsState.process_event(e);
            }
            catch(...)
            {
                UQUAD_HAL_LOG(Error) << "UQuad::MotorsStateMachine::dispatchEvent: exception occurred!";
            }
        });
    }

    template <class FSM, class Event>
    void UQuad::MotorsStateMachine::no_transition(Event const &e, FSM&, int state)
    {
        UQUAD_HAL_LOG(Debug) << "UQuad::MotorsStateMachine::no_transition: from state " << state << " on event " << typeid(e).name();
    }

    template <class FSM>
    void UQuad::MotorsStateMachine::AArmDisarm::operator()(EArmDisarm const&, FSM &fsm, SIdle&, SArming&)
    {
        fsm.uquad.setMotorsPWM(UQuad::MOTOR_PWM_ARM_LEVEL, UQuad::MOTOR_PWM_ARM_LEVEL, UQuad::MOTOR_PWM_ARM_LEVEL, UQuad::MOTOR_PWM_ARM_LEVEL);
        system::error_code e;
        fsm.timer.cancel(e);
        fsm.timer.expires_from_now(std::chrono::seconds(2));
        fsm.timer.async_wait([this, &fsm](system::error_code err)
        {
            if(err)
            {
            }
            else
            {
                fsm.dispatchEvent(ETimer<kTimerArmDisarm>());
            }
        });
    }

    template <class FSM>
    void UQuad::MotorsStateMachine::AArmDisarm::operator()(EArmDisarm const&, FSM &fsm, SRunning&, SDisarming&)
    {
        fsm.uquad.setMotorsPWM(0.0f, 0.0f, 0.0f, 0.0f);
        system::error_code e;
        fsm.timer.cancel(e);
        fsm.timer.expires_from_now(std::chrono::milliseconds(100));
        fsm.timer.async_wait([this, &fsm](system::error_code err)
        {
            fsm.dispatchEvent(ETimer<kTimerArmDisarm>());
        });
    }

    template <class FSM>
    void UQuad::MotorsStateMachine::ARunning::operator()(ESetPower const &esp, FSM &fsm, SRunning&, SRunning&)
    {
        fsm.uquad.setMotorsPWM(
            UQuad::MOTOR_PWM_ARM_LEVEL + esp.fl * UQuad::MOTOR_PWM_RANGE,
            UQuad::MOTOR_PWM_ARM_LEVEL + esp.fr * UQuad::MOTOR_PWM_RANGE,
            UQuad::MOTOR_PWM_ARM_LEVEL + esp.rl * UQuad::MOTOR_PWM_RANGE,
            UQuad::MOTOR_PWM_ARM_LEVEL + esp.rr * UQuad::MOTOR_PWM_RANGE
        );
    }

    float const UQuad::MOTOR_PWM_ARM_LEVEL = 3.0f/255.0f;
    float const UQuad::MOTOR_PWM_RANGE = 1.0f - MOTOR_PWM_ARM_LEVEL;

    UQuad::UQuad()
        : hal::UQuad()
        , m_MotorsState(ref(*this))
        , m_Device(0)
        , m_ReadEp(0)
        , m_WriteEp(0)
        , m_DeviceDescriptor(SI(IOService).ios())
        , m_ReadPacket()
        , m_WritePacket()
        , m_ReadRequest(0)
        , m_WriteRequest(0)
        , m_WriteEvent(SI(IOService).ios())
        , m_WriteGuard()
        , m_WriteDirtyFlags(0)
        , m_SensorsDataGuard()
        , m_SensorsData()
    {
        m_LedsMode[0] = kLedMode0;
        m_LedsMode[1] = kLedMode0;
        m_LedsMode[2] = kLedMode0;
        m_LedsMode[3] = kLedMode0;

        m_MotorsPower[0] = 0.0f;
        m_MotorsPower[1] = 0.0f;
        m_MotorsPower[2] = 0.0f;
        m_MotorsPower[3] = 0.0f;
        m_MotorsState.start();
    }

    UQuad::~UQuad()
    {
        close();
        m_MotorsState.stop();
    }

    std::string const& UQuad::name() const
    {
        static std::string _name = "UQuad";
        return _name;
    }

    bool UQuad::isOpen() const
    {
        return m_Device != 0;
    }

    system::error_code UQuad::open()
    {
        if(isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::open: already opened!";
            return base::makeErrorCode(base::kEAlreadyOpened);
        }

        m_Device = SI(USBHostContext).openDevice(USB_VENDOR_ID, USB_PRODUCT_ID);

        if(!m_Device)
        {
            UQUAD_HAL_LOG(Error) << "UQuad::open: could not open uquad device!";
            return base::makeErrorCode(base::kENoSuchDevice);
        }

        if(!findEndpoints())
        {
            usb_device_close(m_Device);
            m_Device = 0;
            UQUAD_HAL_LOG(Error) << "UQuad::open: could not find endpoints!";
            return base::makeErrorCode(base::kENoSuchDevice);
        }

        if(!startAsyncIOTasks())
        {
            usb_device_close(m_Device);
            m_Device = 0;
            UQUAD_HAL_LOG(Error) << "UQuad::open: could not start async io tasks!";
        }

        m_WriteDirtyFlags = 0;

        setLedsMode(kLedMode0, kLedMode0, kLedMode0, kLedMode0);

        notifyOnDeviceOpened();
        UQUAD_HAL_LOG(Debug) << "UQuad::open: uquad device opened!";
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::close()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::close: uquad device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        stopAsyncIOTasks();

        m_ReadEp = 0;
        m_WriteEp = 0;
        usb_device_close(m_Device);
        m_Device = 0;
        notifyOnDeviceClosed();
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::getSensorsData(UQuadSensorsData &sd) const
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::getSensorsData: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        lock_guard<fast_mutex> lock(m_SensorsDataGuard);
        sd = m_SensorsData;
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::setLedsMode(eLedMode lm0, eLedMode lm1, eLedMode lm2, eLedMode lm3)
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setLedsMode: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        lock_guard<fast_mutex> lock(m_WriteGuard);
        m_LedsMode[0] = lm0;
        m_LedsMode[1] = lm1;
        m_LedsMode[2] = lm2;
        m_LedsMode[3] = lm3;

        m_WriteDirtyFlags |= WRITE_FLAG_LEDS;

        system::error_code err;
        m_WriteEvent.cancel(err);

        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::armMotors()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::armMotors: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        if(!m_MotorsState.is_flag_active<MotorsStateMachine::Flag<MotorsStateMachine::kFlagIdle>>())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::armMotors: motors not idle";
            return base::makeErrorCode(base::kEInvalidState);
        }

        m_MotorsState.dispatchEvent(MotorsStateMachine::EArmDisarm());
        return base::makeErrorCode(base::kENoError);
    }

    bool UQuad::areMotorsRunning() const
    {
        return m_MotorsState.is_flag_active<MotorsStateMachine::Flag<MotorsStateMachine::kFlagRunning>>();
    }

    system::error_code UQuad::disarmMotors()
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::disarmMotors: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        if(!areMotorsRunning())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::disarmMotors: motors not running!";
            return base::makeErrorCode(base::kEInvalidState);
        }

        m_MotorsState.dispatchEvent(MotorsStateMachine::EArmDisarm());
        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::setMotorsPower(float mpfl, float mpfr, float mprl, float mprr)
    {
        if(!isOpen())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setMotorsPower: device not opened!";
            return base::makeErrorCode(base::kENotOpened);
        }

        if(!areMotorsRunning())
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::setMotorsPower: motors not running!";
            return base::makeErrorCode(base::kEInvalidState);
        }

        //this should go trough sm
        //setMotorsPWM(mpfl, mpfr, mprl, mprr);
        m_MotorsState.dispatchEvent(MotorsStateMachine::ESetPower({mpfl, mpfr, mprl, mprr}));
        return base::makeErrorCode(base::kENoError);
    }

    void UQuad::setMotorsPWM(float mpfl, float mpfr, float mprl, float mprr)
    {
        lock_guard<fast_mutex> lock(m_WriteGuard);
        m_MotorsPower[0] = mpfl;
        m_MotorsPower[1] = mpfr;
        m_MotorsPower[2] = mprl;
        m_MotorsPower[3] = mprr;

        m_WriteDirtyFlags |= WRITE_FLAG_MOTORS;

        system::error_code err;
        m_WriteEvent.cancel(err);
    }

    bool UQuad::findEndpoints()
    {
        usb_device_descriptor const * deviceDesc = usb_device_get_device_descriptor(m_Device);
        if(!deviceDesc)
            return false;

        usb_descriptor_iter iter;
        usb_descriptor_header* desc;
        usb_descriptor_iter_init(m_Device, &iter);
        while((desc = usb_descriptor_iter_next(&iter)) != 0)
        {
            if(desc->bDescriptorType == USB_DT_ENDPOINT)
            {
                usb_endpoint_descriptor *endpoint = (usb_endpoint_descriptor *)desc;

                if((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
                {
                    m_ReadEp = endpoint;
                }
                else
                if((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)
                {
                    m_WriteEp = endpoint;
                }
            }
        }

        return m_ReadEp != 0 && m_WriteEp != 0;
    }

    bool UQuad::startAsyncIOTasks()
    {
        system::error_code err;
        m_DeviceDescriptor.assign(usb_device_get_fd(m_Device), err);
        if(err)
        {
            return false;
        }

        m_DeviceDescriptor.non_blocking(true, err);
        if(err)
        {
            return false;
        }

        m_ReadRequest = usb_request_new(m_Device, m_ReadEp);
        if(!m_ReadRequest)
        {
            m_DeviceDescriptor.release();
            return false;
        }

        m_WriteRequest = usb_request_new(m_Device, m_WriteEp);
        if(!m_WriteRequest)
        {
            usb_request_free(m_ReadRequest);
            m_ReadRequest = 0;
            m_DeviceDescriptor.release();
            return false;
        }

        m_ReadRequest->buffer = &m_ReadPacket;
        m_ReadRequest->buffer_length = UQUAD_DEVICE_PKT_SIZE;

        m_WriteRequest->buffer = &m_WritePacket;
        m_WriteRequest->buffer_length = UQUAD_DEVICE_PKT_SIZE;

        if(system::error_code rpe = queueReadRequest())
        {
            usb_request_free(m_ReadRequest);
            usb_request_free(m_WriteRequest);
            m_ReadRequest = 0;
            m_WriteRequest = 0;
            m_DeviceDescriptor.release();
            return false;
        }

        m_DeviceDescriptor.async_write_some(
            asio::null_buffers(),
            bind(
                &UQuad::handleReadWriteRequest,
                this,
                asio::placeholders::error(),
                asio::placeholders::bytes_transferred()
            )
        );

        startWriteEvent();
        return true;
    }

    void UQuad::stopAsyncIOTasks()
    {
        {
            lock_guard<fast_mutex> lock(m_WriteGuard);
            m_WriteDirtyFlags = 0;
            system::error_code err;
            m_WriteEvent.cancel(err);
        }

        usb_request_cancel(m_ReadRequest);
        usb_request_cancel(m_WriteRequest);
        m_DeviceDescriptor.cancel();
        m_DeviceDescriptor.release();

        usb_request_free(m_ReadRequest);
        usb_request_free(m_WriteRequest);
        m_ReadRequest = 0;
        m_WriteRequest = 0;
    }

    system::error_code UQuad::queueReadRequest()
    {
        int rr = usb_request_queue(m_ReadRequest);

        if(rr < 0)
        {
            UQUAD_HAL_LOG(Error) << "UQuad::queueReadRequest: failed to queue read request: " << rr;
            return base::makeErrorCode(base::kENotStarted);
        }

        return base::makeErrorCode(base::kENoError);
    }

    system::error_code UQuad::queueWriteRequest()
    {
        int wr = usb_request_queue(m_WriteRequest);

        if(wr < 0)
        {
            UQUAD_HAL_LOG(Error) << "UQuad::writePacket: failed to queue write request: " << wr;
            return base::makeErrorCode(base::kENotStarted);
        }

        return base::makeErrorCode(base::kENoError);
    }

    void UQuad::handleReadWriteRequest(system::error_code err, std::size_t n)
    {
        if(err)
        {
            if(err != asio::error::operation_aborted)
            {
                UQUAD_HAL_LOG(Error) << "UQuad::handleReadWriteRequest: error occurred: " << err;
            }
        }
        else
        {
            system::error_code waitErr;
            USBHostContext::IOCTLReapUrbNoDelay asyncWaitReq;
            m_DeviceDescriptor.io_control(asyncWaitReq, waitErr);
            if(waitErr)
            {
                UQUAD_HAL_LOG(Error) << "UQuad::handleReadWriteRequest: unexpected IOCTLReapUrbNoDelay error: " << waitErr;
            }
            else
            {
                if(usb_request *req = asyncWaitReq.request())
                {
                    if(req == m_ReadRequest)
                    {
                        handleReadPacket();
                    }
                    else
                    if(req == m_WriteRequest)
                    {
                        handleWritePacket();
                    }
                }
                else
                {
                    UQUAD_HAL_LOG(Error) << "UQuad::handleReadWriteRequest: could not get usb_request";
                }
            }

            m_DeviceDescriptor.async_write_some(
                asio::null_buffers(),
                bind(
                    &UQuad::handleReadWriteRequest,
                    this,
                    asio::placeholders::error(),
                    asio::placeholders::bytes_transferred()
                )
            );
        }
    }

    void UQuad::handleReadPacket()
    {
        lock_guard<fast_mutex> lock(m_SensorsDataGuard);
        m_SensorsData.time = base::TimeClock::now();
        m_SensorsData.velocityRate = Vec3f(
            m_ReadPacket.packet.measurement.acceleration.x/8192.0f,
            m_ReadPacket.packet.measurement.acceleration.y/8192.0f,
            m_ReadPacket.packet.measurement.acceleration.z/8192.0f
        );
        m_SensorsData.rotationRate = Vec3f(
            m_ReadPacket.packet.measurement.rotation_rate.x/65.536f,
            m_ReadPacket.packet.measurement.rotation_rate.y/65.536f,
            m_ReadPacket.packet.measurement.rotation_rate.z/65.536f
        );
        m_SensorsData.magneticField = Vec3f(
            m_ReadPacket.packet.measurement.magnetic_field.x/6.0f,
            m_ReadPacket.packet.measurement.magnetic_field.y/6.0f,
            m_ReadPacket.packet.measurement.magnetic_field.z/6.0f
        );
        m_SensorsData.baroTemperature = m_ReadPacket.packet.measurement.atm_temperature/256.0f;
        m_SensorsData.baroPressure = (m_ReadPacket.packet.measurement.atm_pressure >> 4)/4.0f;

        notifyOnUQuadSensorsReady();
        queueReadRequest();
    }

    void UQuad::handleWritePacket()
    {
        lock_guard<fast_mutex> lock(m_WriteGuard);
        if(!writeDirtyPacket())
        {
            startWriteEvent();
        }
    }

    void UQuad::startWriteEvent()
    {
        system::error_code err;
        m_WriteEvent.expires_at(asio::steady_timer::time_point::max(), err);
        m_WriteEvent.async_wait(
            bind(
                &UQuad::handleWriteEvent,
                this,
                asio::placeholders::error()
            )
        );
    }

    void UQuad::handleWriteEvent(system::error_code err)
    {
        if(!err)
        {
            UQUAD_HAL_LOG(Warning) << "UQuad::handleWriteEvent: write event should not timeout";
            lock_guard<fast_mutex> lock(m_WriteGuard);
            startWriteEvent();
            return;
        }

        if(err && err != asio::error::operation_aborted)
        {
            UQUAD_HAL_LOG(Error) << "UQuad::handleWriteEvent: unexpected write event error:" << err;
            return;
        }

        lock_guard<fast_mutex> lock(m_WriteGuard);
        if(!writeDirtyPacket())
        {
            startWriteEvent();
        }
    }

    bool UQuad::writeDirtyPacket()
    {
        while(m_WriteDirtyFlags)
        {
            if(m_WriteDirtyFlags & WRITE_FLAG_MOTORS)
            {
                m_WriteDirtyFlags &= ~WRITE_FLAG_MOTORS;
                if(!writeMotorsPowerPacket())
                {
                    return true;
                }
            }

            if(m_WriteDirtyFlags & WRITE_FLAG_LEDS)
            {
                m_WriteDirtyFlags &= ~WRITE_FLAG_LEDS;
                if(!writeLedsModePacket())
                {
                    return true;
                }
            }
        }

        return false;
    }

    system::error_code UQuad::writeLedsModePacket()
    {
        m_WritePacket.packet.id = UQUAD_DEVICE_PKT_LEDS_MODE;
        m_WritePacket.packet.leds_mode.led0_mode = m_LedsMode[0];
        m_WritePacket.packet.leds_mode.led1_mode = m_LedsMode[1];
        m_WritePacket.packet.leds_mode.led2_mode = m_LedsMode[2];
        m_WritePacket.packet.leds_mode.led3_mode = m_LedsMode[3];
        return queueWriteRequest();
    }

    system::error_code UQuad::writeMotorsPowerPacket()
    {
        m_WritePacket.packet.id = UQUAD_DEVICE_PKT_MOTORS_PWM;
        m_WritePacket.packet.motors_pwm.motor0_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m_MotorsPower[0])));
        m_WritePacket.packet.motors_pwm.motor1_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m_MotorsPower[1])));
        m_WritePacket.packet.motors_pwm.motor2_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m_MotorsPower[2])));
        m_WritePacket.packet.motors_pwm.motor3_pwm = static_cast<uint8_t>(0.5f + 255.0f*std::max(0.0f, std::min(1.0f, m_MotorsPower[3])));
        return queueWriteRequest();
    }



} //namespace android
} //namespace hal
} //namespace uquad
