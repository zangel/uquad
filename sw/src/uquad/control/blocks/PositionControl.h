#ifndef UQUAD_CONTROL_BLOCKS_POSITION_CONTROL_H
#define UQUAD_CONTROL_BLOCKS_POSITION_CONTROL_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
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
        
        PositionControl();
        ~PositionControl();
        
        intrusive_ptr<InputSignal> inputSignalDT() const;
        intrusive_ptr<InputSignal> inputSignalAttitudeEstimated() const;
        intrusive_ptr<InputSignal> inputSignalPositionEstimated() const;
        intrusive_ptr<InputSignal> inputSignalVelocityEstimated() const;
        intrusive_ptr<InputSignal> inputSignalThrustControl() const;
        intrusive_ptr<InputSignal> inputSignalDirectionControl() const;
        intrusive_ptr<OutputSignal> outputSignalThrustSetpoint() const;
        intrusive_ptr<OutputSignal> outputSignalAttitudeSetpoint() const;

    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        mutable DefaultInputSignal<DT> m_ISDT;
        mutable DefaultInputSignal<Attitude> m_ISAttitudeEstimated;
        mutable DefaultInputSignal<Position> m_ISPositionEstimated;
        mutable DefaultInputSignal<Velocity> m_ISVelocityEstimated;
        mutable DefaultInputSignal<Thrust> m_ISThrustControl;
        mutable DefaultInputSignal<Direction> m_ISDirectionControl;
        
        mutable DefaultOutputSignal<Thrust> m_OSThrustSetpoint;
        mutable DefaultOutputSignal<Attitude> m_OSAttitudeSetpoint;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_POSITION_CONTROL_H
