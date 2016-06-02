#ifndef UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_H
#define UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class AttitudeControl
        : public virtual Block
    {
    public:
        typedef float DT;
        typedef Quaternionf Attitude;
        typedef Vec3f RotationRate;
        
        AttitudeControl();
        ~AttitudeControl();
        
        intrusive_ptr<InputSignal> inputSignalDT() const;
        intrusive_ptr<InputSignal> inputSignalAttitudeSetpoint() const;
        intrusive_ptr<InputSignal> inputSignalAttitudeEstimated() const;
        intrusive_ptr<InputSignal> inputSignalRotationRateMeasured() const;
        intrusive_ptr<OutputSignal> outputSignalRotationRateSetpoint() const;
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        mutable DefaultInputSignal<DT> m_ISDT;
        mutable DefaultInputSignal<Attitude> m_ISAttitudeSetpoint;
        mutable DefaultInputSignal<Attitude> m_ISAttitudeEstimated;
        mutable DefaultInputSignal<RotationRate> m_ISRotationRateMeasured;
        mutable DefaultOutputSignal<RotationRate> m_OSRotationRateSetpoint;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_H
