#include "QuadMotorsThrust.h"

namespace uquad
{
namespace control
{
    QuadMotorsThrust::QuadMotorsThrust()
        : MotorsThrust()
        , m_ThrustSetpoint(thrustSetpointName(), 0.0f)
        , m_RotationRateSetpoint(rotationRateSetpointName(), RotationRate::Zero())
        , m_Motors(motorsName(), Motors::Zero())
    {
        intrusive_ptr_add_ref(&m_ThrustSetpoint); addInputPort(&m_ThrustSetpoint);
        intrusive_ptr_add_ref(&m_RotationRateSetpoint); addInputPort(&m_RotationRateSetpoint);
        intrusive_ptr_add_ref(&m_Motors); addOutputPort(&m_Motors);
    }
    
    QuadMotorsThrust::~QuadMotorsThrust()
    {
    }
    
    Vec4f QuadMotorsThrust::getMotors() const
    {
        return m_Motors.m_Value;
    }
    
    system::error_code QuadMotorsThrust::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = MotorsThrust::prepare(yctx))
            return bpe;
        
        m_ThrustSetpoint.m_Value = 0.0f;
        m_RotationRateSetpoint.m_Value = RotationRate::Zero();
        m_Motors.m_Value = Motors::Zero();
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad