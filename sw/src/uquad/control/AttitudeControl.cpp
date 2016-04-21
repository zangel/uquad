#include "AttitudeControl.h"

namespace uquad
{
namespace control
{
    std::string const& AttitudeControl::dTName()
    {
        static std::string _name = "dT";
        return _name;
    }
    
    std::string const& AttitudeControl::attitudeName()
    {
        static std::string _name = "attitude";
        return _name;
    }
    
    std::string const& AttitudeControl::attitudeSetpointName()
    {
        static std::string _name = "attitudeSetpoint";
        return _name;
    }
    
    std::string const& AttitudeControl::rotationRateName()
    {
        static std::string _name = "rotationRate";
        return _name;
    }
    
    std::string const& AttitudeControl::rotationRateSetpointName()
    {
        static std::string _name = "rotationRateSetpoint";
        return _name;
    }
    
    AttitudeControl::AttitudeControl()
        : Block()
        , m_DT(dTName(), 0.0f)
        , m_Attitude(attitudeName(), Attitude::Identity())
        , m_AttitudeSetpoint(attitudeSetpointName(), Attitude::Identity())
        , m_RotationRate(rotationRateName(), RotationRate::Zero())
        , m_RotationRateSetpoint(rotationRateSetpointName(), RotationRate::Zero())
    {
        intrusive_ptr_add_ref(&m_DT); addInputPort(&m_DT);
        intrusive_ptr_add_ref(&m_Attitude); addInputPort(&m_Attitude);
        intrusive_ptr_add_ref(&m_AttitudeSetpoint); addInputPort(&m_AttitudeSetpoint);
        intrusive_ptr_add_ref(&m_RotationRate); addInputPort(&m_RotationRate);
        intrusive_ptr_add_ref(&m_RotationRateSetpoint); addOutputPort(&m_RotationRateSetpoint);
    }
    
    AttitudeControl::~AttitudeControl()
    {
    }
    
    system::error_code AttitudeControl::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_DT.m_Value = 0.0f;
        m_Attitude.m_Value = Attitude::Identity();
        m_AttitudeSetpoint.m_Value = Attitude::Identity();
        m_RotationRate.m_Value = RotationRate::Zero();
        m_RotationRateSetpoint.m_Value = RotationRate::Zero();
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad
