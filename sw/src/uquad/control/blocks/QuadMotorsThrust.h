#ifndef UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_H
#define UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_H

#include "../Config.h"
#include "MotorsThrust.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class QuadMotorsThrust
        : public MotorsThrust
    {
    public:
        typedef float Thrust;
        typedef Vec3f RotationRate;
        typedef Vec4f MotorsPower;
        
        QuadMotorsThrust();
        ~QuadMotorsThrust();
        
        MotorsPower getMotorsPower() const;
        
        intrusive_ptr<InputSignal> inputSignalThrustSetpoint() const;
        intrusive_ptr<InputSignal> inputSignalRotationRateSetpoint() const;
        intrusive_ptr<OutputSignal> outputSignalMotorsPower() const;
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        mutable DefaultInputSignal<Thrust> m_ISThrustSetpoint;
        mutable DefaultInputSignal<RotationRate> m_ISRotationRateSetpoint;
        mutable DefaultOutputSignal<MotorsPower> m_OSMotorsPower;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_QUAD_MOTORS_THRUST_H
