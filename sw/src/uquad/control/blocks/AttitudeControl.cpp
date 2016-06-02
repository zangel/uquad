#include "AttitudeControl.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    AttitudeControl::AttitudeControl()
        : Block()
        , m_ISDT("dT", 0.0f)
        , m_ISAttitudeSetpoint("attitudeSetpoint", Attitude::Identity())
        , m_ISAttitudeEstimated("attitudeEstimated", Attitude::Identity())
        , m_ISRotationRateMeasured("rotationRateMeasured", RotationRate::Zero())
        , m_OSRotationRateSetpoint("rotationRateSetpoint", RotationRate::Zero())
    {
        intrusive_ptr_add_ref(&m_ISDT); addInputPort(&m_ISDT);
        intrusive_ptr_add_ref(&m_ISAttitudeSetpoint); addInputPort(&m_ISAttitudeSetpoint);
        intrusive_ptr_add_ref(&m_ISAttitudeEstimated); addInputPort(&m_ISAttitudeEstimated);
        intrusive_ptr_add_ref(&m_ISRotationRateMeasured); addInputPort(&m_ISRotationRateMeasured);
        intrusive_ptr_add_ref(&m_OSRotationRateSetpoint); addOutputPort(&m_OSRotationRateSetpoint);
    }
    
    AttitudeControl::~AttitudeControl()
    {
    }
    
    intrusive_ptr<InputSignal> AttitudeControl::inputSignalDT() const
    {
        return &m_ISDT;
    }
    
    intrusive_ptr<InputSignal> AttitudeControl::inputSignalAttitudeSetpoint() const
    {
        return &m_ISAttitudeSetpoint;
    }
    
    intrusive_ptr<InputSignal> AttitudeControl::inputSignalAttitudeEstimated() const
    {
        return &m_ISAttitudeEstimated;
    }
    
    intrusive_ptr<InputSignal> AttitudeControl::inputSignalRotationRateMeasured() const
    {
        return &m_ISRotationRateMeasured;
    }
    
    intrusive_ptr<OutputSignal> AttitudeControl::outputSignalRotationRateSetpoint() const
    {
        return &m_OSRotationRateSetpoint;
    }
    
    
    system::error_code AttitudeControl::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_ISDT.m_Value = 0.0f;
        m_ISAttitudeSetpoint.m_Value = Attitude::Identity();
        m_ISAttitudeEstimated.m_Value = Attitude::Identity();
        m_ISRotationRateMeasured.m_Value = RotationRate::Zero();
        m_OSRotationRateSetpoint.m_Value = RotationRate::Zero();
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace blocks    
} //namespace control
} //namespace uquad
