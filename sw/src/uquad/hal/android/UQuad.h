#ifndef UQUAD_HAL_ANDROID_UQUAD_H
#define UQUAD_HAL_ANDROID_UQUAD_H

#include "../Config.h"
#include "../UQuad.h"
#include <usbhost/usbhost.h>
#include <uQuad.X/uquad_device_types.h>

namespace uquad
{
namespace hal
{
namespace android
{
    class UQuad
    	: public hal::UQuad
    {
    public:
        friend class MotorsStateMachine;

        struct MotorsStateMachine : public msm::front::state_machine_def<MotorsStateMachine>
        {
            typedef enum
            {
                kFlagIdle,
                kFlagRunning,
                kFlagCalibrating
            } eFlag;

            template < eFlag > struct Flag {};

            struct EArmDisarm {};
            struct ECalibrate {};
            struct ESetPower
            {
                float fl, fr, rl, rr;
            };

            typedef enum
            {
                kTimerArmDisarm
            } eTimer;

            template < eTimer > struct ETimer {};

            struct SIdle : public msm::front::state<>
            {
                typedef mpl::vector1< Flag<kFlagIdle> > flag_list;
            };

            struct SArming : public msm::front::state<>
            {
            };

            struct SRunning : public msm::front::state<>
            {
                typedef mpl::vector1< Flag<kFlagRunning> > flag_list;
            };

            struct SDisarming : public msm::front::state<>
            {
            };

            struct SCalibrating : public msm::front::state<>
            {
                typedef mpl::vector1< Flag<kFlagCalibrating> > flag_list;
            };

            struct AArmDisarm
            {
                template <class FSM>
                void operator()(EArmDisarm const&, FSM &fsm, SIdle&, SArming&);

                template <class FSM>
                void operator()(EArmDisarm const&, FSM &fsm, SRunning&, SDisarming&);
            };

            struct ARunning
            {
                template <class FSM>
                void operator()(ESetPower const&, FSM &fsm, SRunning&, SRunning&);
            };

            typedef SIdle initial_state;

            struct transition_table : mpl::vector
            <
            //             |   |Start           |Event                      |Next           |Action             |
            //             ----+----------------+---------------------------+---------------+-------------------|
                msm::front::Row<SIdle           ,EArmDisarm                 ,SArming        ,AArmDisarm         >,
                msm::front::Row<SArming         ,ETimer<kTimerArmDisarm>    ,SRunning       ,msm::front::none   >,
                msm::front::Row<SRunning        ,EArmDisarm                 ,SDisarming     ,AArmDisarm         >,
                msm::front::Row<SRunning        ,ESetPower                  ,SRunning       ,ARunning           >,
                msm::front::Row<SDisarming      ,ETimer<kTimerArmDisarm>    ,SIdle          ,msm::front::none   >
            > {};


            MotorsStateMachine(UQuad &uq);

            template <class FSM, class Event>
            void no_transition(Event const &e, FSM&, int state);

            template <typename Event>
            void dispatchEvent(Event e);

            UQuad &uquad;
            asio::steady_timer timer;
        };

        typedef msm::back::state_machine<MotorsStateMachine>  MotorsState;

        static uint16_t const USB_VENDOR_ID     = 0x3562;
        static uint16_t const USB_PRODUCT_ID    = 0x0001;

        static uint32_t const WRITE_FLAG_LEDS   = 0x00000001;
        static uint32_t const WRITE_FLAG_MOTORS = 0x00000002;

        static float const MOTOR_PWM_ARM_LEVEL;
        static float const MOTOR_PWM_RANGE;


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

        void setMotorsPWM(float mpfl, float mpfr, float mprl, float mprr);

    private:
        bool findEndpoints();
        bool startAsyncIOTasks();
        void stopAsyncIOTasks();

        system::error_code queueReadRequest();
        system::error_code queueWriteRequest();
        void handleReadWriteRequest(system::error_code err, std::size_t n);

        void handleReadPacket();
        void handleWritePacket();

        void startWriteEvent();
        void handleWriteEvent(system::error_code err);

        bool writeDirtyPacket();
        system::error_code writeLedsModePacket();
        system::error_code writeMotorsPowerPacket();


    private:
        MotorsState m_MotorsState;
        usb_device *m_Device;
        usb_endpoint_descriptor *m_ReadEp;
        usb_endpoint_descriptor *m_WriteEp;

        asio::posix::stream_descriptor m_DeviceDescriptor;

        uquad_device_packet_t m_ReadPacket;
        uquad_device_packet_t m_WritePacket;

        usb_request *m_ReadRequest;
        usb_request *m_WriteRequest;

        asio::steady_timer m_WriteEvent;
        mutable fast_mutex m_WriteGuard;
        uint32_t m_WriteDirtyFlags;

        eLedMode m_LedsMode[4];
        float m_MotorsPower[4];

        mutable fast_mutex m_SensorsDataGuard;
        UQuadSensorsData m_SensorsData;
    };

} //namespace android
} //namespace hal
} //namespace uquad


#endif //UQUAD_HAL_ANDROID_UQUAD_H
