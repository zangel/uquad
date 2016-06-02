#include "Gyroscope.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    Gyroscope::Gyroscope()
        : Block()
        , m_OSRotationRate("rotationRate", RotationRate::Zero())
    {
        intrusive_ptr_add_ref(&m_OSRotationRate); addOutputPort(&m_OSRotationRate);
    }
    
    Gyroscope::~Gyroscope()
    {
    }
    
    intrusive_ptr<OutputSignal> Gyroscope::outputSignalRotationRate() const
    {
        return &m_OSRotationRate;
    }
    
} //namespace blocks        
} //namespace control
} //namespace uquad
