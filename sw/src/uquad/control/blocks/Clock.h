#ifndef UQUAD_CONTROL_BLOCKS_CLOCK_H
#define UQUAD_CONTROL_BLOCKS_CLOCK_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class Clock
        : public virtual Block
    {
    public:
        typedef float Time;
        typedef float DT;
        
        Clock();
        ~Clock();
        
        intrusive_ptr<OutputSignal> outputSignalTime() const;
        intrusive_ptr<OutputSignal> outputSignalDT() const;
        
    protected:
        mutable DefaultOutputSignal<Time> m_OSTime;
        mutable DefaultOutputSignal<DT> m_OSDT;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_CLOCK_H
