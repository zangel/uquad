#ifndef UQUAD_CONTROL_BLOCKS_GYROSCOPE_H
#define UQUAD_CONTROL_BLOCKS_GYROSCOPE_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class Gyroscope
        : public virtual Block
    {
    public:
        typedef Vec3f RotationRate;
        
        Gyroscope();
        ~Gyroscope();
        
        intrusive_ptr<OutputSignal> outputSignalRotationRate() const;
        
    protected:
        
    protected:
        mutable DefaultOutputSignal<RotationRate> m_OSRotationRate;
    };

} //namespace blocks    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_GYROSCOPE_H
