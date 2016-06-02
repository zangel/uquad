#ifndef UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_H
#define UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class ManualControl
        : public virtual Block
    {
    public:
        typedef Vec3f Direction;
        typedef float Thrust;
        
        ManualControl();
        ~ManualControl();
        
        intrusive_ptr<OutputSignal> outputSignalDirection() const;
        intrusive_ptr<OutputSignal> outputSignalThrust() const;
        
    protected:
        mutable DefaultOutputSignal<Direction> m_OSDirection;
        mutable DefaultOutputSignal<Thrust> m_OSThrust;
    };

} //namespace blocks    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_MANUAL_CONTROL_H
