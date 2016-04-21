#include "PositionControl.h"

namespace uquad
{
namespace control
{
    std::string const& PositionControl::dTName()
    {
        static std::string _name = "dT";
        return _name;
    }
    
    std::string const& PositionControl::attitudeName()
    {
        static std::string _name = "attitude";
        return _name;
    }
    
    
    std::string const& PositionControl::positionName()
    {
        static std::string _name = "position";
        return _name;
    }
    
    std::string const& PositionControl::velocityName()
    {
        static std::string _name = "velocity";
        return _name;
    }
    
    std::string const& PositionControl::thrustControlName()
    {
        static std::string _name = "thrustControl";
        return _name;
    }
    
    std::string const& PositionControl::directionControlName()
    {
        static std::string _name = "directionControl";
        return _name;
    }
    
    std::string const& PositionControl::thrustSetpointName()
    {
        static std::string _name = "thrustSetpoint";
        return _name;
    }
    
    std::string const& PositionControl::attitudeSetpointName()
    {
        static std::string _name = "attitudeSetpoint";
        return _name;
    }
    
    
    
    PositionControl::PositionControl()
        : Block()
        , m_DT(dTName(), 0.0f)
        , m_Attitude(attitudeName(), Attitude::Identity())
        , m_Position(positionName(), Vec3f::Zero())
        , m_Velocity(velocityName(), Vec3f::Zero())
        , m_ThrustControl(thrustControlName(), 0.0f)
        , m_DirectionControl(directionControlName(), Vec3f::Zero())
        , m_ThrustSetpoint(thrustSetpointName(), 0.0f)
        , m_AttitudeSetpoint(attitudeSetpointName(), Attitude::Identity())
    {
        intrusive_ptr_add_ref(&m_DT); addInputPort(&m_DT);
        intrusive_ptr_add_ref(&m_Attitude); addInputPort(&m_Attitude);
        intrusive_ptr_add_ref(&m_Position); addInputPort(&m_Position);
        intrusive_ptr_add_ref(&m_Velocity); addInputPort(&m_Velocity);
        
        intrusive_ptr_add_ref(&m_ThrustControl); addInputPort(&m_ThrustControl);
        intrusive_ptr_add_ref(&m_DirectionControl); addInputPort(&m_DirectionControl);
        
        intrusive_ptr_add_ref(&m_ThrustSetpoint); addOutputPort(&m_ThrustSetpoint);
        intrusive_ptr_add_ref(&m_AttitudeSetpoint); addOutputPort(&m_AttitudeSetpoint);
    }
    
    PositionControl::~PositionControl()
    {
    }
    
    system::error_code PositionControl::prepare(asio::yield_context yctx)
    {
        if(system::error_code bpe = Block::prepare(yctx))
            return bpe;
        
        m_DT.m_Value = 0.0f;
        m_Position.m_Value = Vec3f::Zero();
        m_Velocity.m_Value = Vec3f::Zero();
        m_ThrustControl.m_Value = 0.0f;
        m_DirectionControl.m_Value = Vec3f::Zero();
        
        m_ThrustSetpoint.m_Value = 0.0f;
        m_AttitudeSetpoint.m_Value = Attitude::Identity();
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad
