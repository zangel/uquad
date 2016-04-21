#ifndef UQUAD_CONTROL_POSITION_CONTROL_H
#define UQUAD_CONTROL_POSITION_CONTROL_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class PositionControl
        : public virtual Block
    {
    public:
        typedef float DT;
        typedef Quaternionf Attitude;
        typedef Vec3f Position;
        typedef Vec3f Velocity;
        typedef float Thrust;
        typedef Vec3f Direction;
        
        static std::string const& dTName();
        static std::string const& positionName();
        static std::string const& velocityName();
        static std::string const& attitudeName();
        
        static std::string const& thrustControlName();
        static std::string const& directionControlName();
        
        static std::string const& attitudeSetpointName();
        static std::string const& thrustSetpointName();
        
        PositionControl();
        ~PositionControl();
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        DefaultInputSignal<DT> m_DT;
        DefaultInputSignal<Attitude> m_Attitude;
        DefaultInputSignal<Position> m_Position;
        DefaultInputSignal<Velocity> m_Velocity;
        DefaultInputSignal<Thrust> m_ThrustControl;
        DefaultInputSignal<Direction> m_DirectionControl;
        
        
        DefaultOutputSignal<Thrust> m_ThrustSetpoint;
        DefaultOutputSignal<Attitude> m_AttitudeSetpoint;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_POSITION_CONTROL_H
