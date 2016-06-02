#include "PositionControl.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    PositionControl::PositionControl()
        : Block()
        , m_ISDT("dT", 0.0f)
        , m_ISAttitudeEstimated("attitudeEstimated", Attitude::Identity())
        , m_ISPositionEstimated("positionEstimated", Vec3f::Zero())
        , m_ISVelocityEstimated("velocityEstimated", Vec3f::Zero())
        , m_ISThrustControl("thrustControl", 0.0f)
        , m_ISDirectionControl("directionControl", Vec3f::Zero())
        , m_OSThrustSetpoint("thrustSetpoint", 0.0f)
        , m_OSAttitudeSetpoint("attitudeSetpoint", Attitude::Identity())
    {
        intrusive_ptr_add_ref(&m_ISDT); addInputPort(&m_ISDT);
        intrusive_ptr_add_ref(&m_ISAttitudeEstimated); addInputPort(&m_ISAttitudeEstimated);
        intrusive_ptr_add_ref(&m_ISPositionEstimated); addInputPort(&m_ISPositionEstimated);
        intrusive_ptr_add_ref(&m_ISVelocityEstimated); addInputPort(&m_ISVelocityEstimated);
        
        intrusive_ptr_add_ref(&m_ISThrustControl); addInputPort(&m_ISThrustControl);
        intrusive_ptr_add_ref(&m_ISDirectionControl); addInputPort(&m_ISDirectionControl);
        
        intrusive_ptr_add_ref(&m_OSThrustSetpoint); addOutputPort(&m_OSThrustSetpoint);
        intrusive_ptr_add_ref(&m_OSAttitudeSetpoint); addOutputPort(&m_OSAttitudeSetpoint);
    }
    
    PositionControl::~PositionControl()
    {
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalDT() const
    {
        return &m_ISDT;
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalAttitudeEstimated() const
    {
        return &m_ISAttitudeEstimated;
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalPositionEstimated() const
    {
        return &m_ISPositionEstimated;
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalVelocityEstimated() const
    {
        return &m_ISVelocityEstimated;
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalThrustControl() const
    {
        return &m_ISThrustControl;
    }
    
    intrusive_ptr<InputSignal> PositionControl::inputSignalDirectionControl() const
    {
        return &m_ISDirectionControl;
    }
    
    intrusive_ptr<OutputSignal> PositionControl::outputSignalThrustSetpoint() const
    {
        return &m_OSThrustSetpoint;
    }
    
    intrusive_ptr<OutputSignal> PositionControl::outputSignalAttitudeSetpoint() const
    {
        return &m_OSAttitudeSetpoint;
    }
    
    system::error_code PositionControl::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_ISDT.m_Value = 0.0f;
        m_ISAttitudeEstimated.m_Value = Attitude::Identity();
        m_ISPositionEstimated.m_Value = Vec3f::Zero();
        m_ISVelocityEstimated.m_Value = Vec3f::Zero();
        m_ISThrustControl.m_Value = 0.0f;
        m_ISDirectionControl.m_Value = Vec3f::Zero();
        
        m_OSThrustSetpoint.m_Value = 0.0f;
        m_OSAttitudeSetpoint.m_Value = Attitude::Identity();
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad
