#ifndef UQUAD_CONTROL_BLOCKS_POSE_ESTIMATION_H
#define UQUAD_CONTROL_BLOCKS_POSE_ESTIMATION_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class PoseEstimation
        : public virtual Block
    {
    public:
        class Registry
            : public base::Object::Registry
        {
        public:
            Registry(std::string const &n);
        };
        
        typedef float DT;
        typedef float Time;
        
        typedef Vec3f VelocityRate;
        typedef Vec3f RotationRate;
        typedef Vec3f MagneticField;
        typedef float Altitude;
        
        typedef Quaternionf Attitude;
        typedef Vec3f Position;
        typedef Vec3f Velocity;
        
        PoseEstimation();
        ~PoseEstimation();
        
        intrusive_ptr<InputSignal> inputSignalDT() const;
        intrusive_ptr<InputSignal> inputSignalTime() const;
        intrusive_ptr<InputSignal> inputSignalVelocityRate() const;
        intrusive_ptr<InputSignal> inputSignalRotationRate() const;
        intrusive_ptr<InputSignal> inputSignalMagneticField() const;
        intrusive_ptr<InputSignal> inputSignalAltitude() const;
        
        intrusive_ptr<OutputSignal> outputSignalAttitude() const;
        intrusive_ptr<OutputSignal> outputSignalPosition() const;
        intrusive_ptr<OutputSignal> outputSignalVelocity() const;
        intrusive_ptr<OutputSignal> outputSignalEarthMagneticField() const;
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        mutable DefaultInputSignal<DT> m_ISDT;
        mutable DefaultInputSignal<Time> m_ISTime;
        mutable DefaultInputSignal<VelocityRate> m_ISVelocityRate;
        mutable DefaultInputSignal<RotationRate> m_ISRotationRate;
        mutable DefaultInputSignal<MagneticField> m_ISMagneticField;
        mutable DefaultInputSignal<Altitude> m_ISAltitude;

        mutable DefaultOutputSignal<Attitude> m_OSAttitude;
        mutable DefaultOutputSignal<Position> m_OSPosition;
        mutable DefaultOutputSignal<Velocity> m_OSVelocity;
        mutable DefaultOutputSignal<MagneticField> m_OSEarthMagneticField;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_POSE_ESTIMATION_H
