#ifndef UQUAD_CONTROL_BLOCKS_ACCELEROMETER_H
#define UQUAD_CONTROL_BLOCKS_ACCELEROMETER_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class Accelerometer
        : public virtual Block
    {
    public:
        typedef Vec3f VelocityRate;
        
        Accelerometer();
        ~Accelerometer();
        
        intrusive_ptr<OutputSignal> outputSignalVelocityRate() const;
        
    protected:
        
    protected:
        mutable DefaultOutputSignal<VelocityRate> m_OSVelocityRate;
    };
    
} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_ACCELEROMETER_H
