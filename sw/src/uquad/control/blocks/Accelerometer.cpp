#include "Accelerometer.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    Accelerometer::Accelerometer()
        : Block()
        , m_OSVelocityRate("velocityRate", VelocityRate::Zero())
    {
        intrusive_ptr_add_ref(&m_OSVelocityRate); addOutputPort(&m_OSVelocityRate);
    }
    
    Accelerometer::~Accelerometer()
    {
    }
    
    intrusive_ptr<OutputSignal> Accelerometer::outputSignalVelocityRate() const
    {
        return &m_OSVelocityRate;
    }

} //namespace blocks
} //namespace control
} //namespace uquad
