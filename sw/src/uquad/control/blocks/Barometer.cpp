#include "Barometer.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    Barometer::Barometer()
        : Block()
        , m_OSPressure("pressure", 0.0f)
        , m_OSAltitude("altitude", 0.0f)
        , m_OSTemperature("temperature", 0.0f)
    {
        intrusive_ptr_add_ref(&m_OSPressure); addOutputPort(&m_OSPressure);
        intrusive_ptr_add_ref(&m_OSAltitude); addOutputPort(&m_OSAltitude);
        intrusive_ptr_add_ref(&m_OSTemperature); addOutputPort(&m_OSTemperature);
    }
    
    Barometer::~Barometer()
    {
    }
    
    intrusive_ptr<OutputSignal> Barometer::outputSignalPressure() const
    {
        return &m_OSPressure;
    }
    
    intrusive_ptr<OutputSignal> Barometer::outputSignalAltitude() const
    {
        return &m_OSAltitude;
    }
    
    intrusive_ptr<OutputSignal> Barometer::outputSignalTemperature() const
    {
        return &m_OSTemperature;
    }

} //namespace blocks
} //namespace control
} //namespace uquad
