#ifndef UQUAD_CONTROL_BLOCKS_MAGNETOMETER_H
#define UQUAD_CONTROL_BLOCKS_MAGNETOMETER_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class Magnetometer
        : public virtual Block
    {
    public:
        typedef Vec3f MagneticField;
        
        Magnetometer();
        ~Magnetometer();
        
        intrusive_ptr<OutputSignal> outputSignalMagneticField() const;
        
    protected:
        mutable DefaultOutputSignal<MagneticField> m_OSMagneticField;
    };

} //namespace blocks    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_MAGNETOMETER_H
