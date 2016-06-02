#include "ManualControl.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    ManualControl::ManualControl()
        : Block()
        , m_OSDirection("direction", Direction::Zero())
        , m_OSThrust("thrust", 0.0f)
    {
        intrusive_ptr_add_ref(&m_OSDirection); addOutputPort(&m_OSDirection);
        intrusive_ptr_add_ref(&m_OSThrust); addOutputPort(&m_OSThrust);
    }
    
    ManualControl::~ManualControl()
    {
    }
    
    intrusive_ptr<OutputSignal> ManualControl::outputSignalDirection() const
    {
        return &m_OSDirection;
    }
    
    intrusive_ptr<OutputSignal> ManualControl::outputSignalThrust() const
    {
        return &m_OSThrust;
    }

} //namespace blocks    
} //namespace control
} //namespace uquad
