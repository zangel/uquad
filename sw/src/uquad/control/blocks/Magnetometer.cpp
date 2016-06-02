#include "Magnetometer.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    Magnetometer::Magnetometer()
        : Block()
        , m_OSMagneticField("magneticField", MagneticField::Zero())
    {
        intrusive_ptr_add_ref(&m_OSMagneticField); addOutputPort(&m_OSMagneticField);
    }
    
    Magnetometer::~Magnetometer()
    {
    }
    
    intrusive_ptr<OutputSignal> Magnetometer::outputSignalMagneticField() const
    {
        return &m_OSMagneticField;
    }

} //namespace blocks    
} //namespace control
} //namespace uquad
