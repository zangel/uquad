#include "QuadMotorsThrust.h"

namespace uquad
{
namespace control
{
namespace blocks
{

    QuadMotorsThrust::QuadMotorsThrust()
        : MotorsThrust()
        , m_ISThrustSetpoint("thrustSetpont", 0.0f)
        , m_ISRotationRateSetpoint("rotationRateSetpoint", RotationRate::Zero())
        , m_OSMotorsPower("motorsPower", MotorsPower::Zero())
    {
        intrusive_ptr_add_ref(&m_ISThrustSetpoint); addInputPort(&m_ISThrustSetpoint);
        intrusive_ptr_add_ref(&m_ISRotationRateSetpoint); addInputPort(&m_ISRotationRateSetpoint);
        intrusive_ptr_add_ref(&m_OSMotorsPower); addOutputPort(&m_OSMotorsPower);
    }
    
    QuadMotorsThrust::~QuadMotorsThrust()
    {
    }
    
    QuadMotorsThrust::MotorsPower QuadMotorsThrust::getMotorsPower() const
    {
        return m_OSMotorsPower.m_Value;
    }
    
    system::error_code QuadMotorsThrust::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = MotorsThrust::prepare(yctx))
            return bpe;
        
        m_ISThrustSetpoint.m_Value = 0.0f;
        m_ISRotationRateSetpoint.m_Value = RotationRate::Zero();
        m_OSMotorsPower.m_Value = MotorsPower::Zero();
        return base::makeErrorCode(base::kENoError);
    }
    
    intrusive_ptr<InputSignal> QuadMotorsThrust::inputSignalThrustSetpoint() const
    {
        return &m_ISThrustSetpoint;
    }
    
    intrusive_ptr<InputSignal> QuadMotorsThrust::inputSignalRotationRateSetpoint() const
    {
        return &m_ISRotationRateSetpoint;
    }
    
    intrusive_ptr<OutputSignal> QuadMotorsThrust::outputSignalMotorsPower() const
    {
        return &m_OSMotorsPower;
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad