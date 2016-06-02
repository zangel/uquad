#include "Clock.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    Clock::Clock()
        : Block()
        , m_OSTime("time", 0.0f)
        , m_OSDT("dT", 0.0f)
    {
        intrusive_ptr_add_ref(&m_OSTime); addOutputPort(&m_OSTime);
        intrusive_ptr_add_ref(&m_OSDT); addOutputPort(&m_OSDT);
    }
    
    Clock::~Clock()
    {
    }
    
    intrusive_ptr<OutputSignal> Clock::outputSignalTime() const
    {
        return &m_OSTime;
    }
    
    intrusive_ptr<OutputSignal> Clock::outputSignalDT() const
    {
        return &m_OSDT;
    }
    
} //namespace blocks
} //namespace control
} //namespace uquad
