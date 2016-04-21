#include "Barometer.h"

namespace uquad
{
namespace control
{
    std::string const& Barometer::pressureName()
    {
        static std::string _name = "pressure";
        return _name;
    }
    
    std::string const& Barometer::relativeAltitudeName()
    {
        static std::string _name = "relativeAltitude";
        return _name;
    }
    
    std::string const& Barometer::temperatureName()
    {
        static std::string _name = "temperature";
        return _name;
    }
    
    Barometer::Barometer()
        : Block()
        , m_Pressure(pressureName(), 0.0f)
        , m_RelativeAltitude(relativeAltitudeName(), 0.0f)
        , m_Temperature(temperatureName(), 0.0f)
    {
        intrusive_ptr_add_ref(&m_Pressure); addOutputPort(&m_Pressure);
        intrusive_ptr_add_ref(&m_RelativeAltitude); addOutputPort(&m_RelativeAltitude);
        intrusive_ptr_add_ref(&m_Temperature); addOutputPort(&m_Temperature);
    }
    
    Barometer::~Barometer()
    {
    }

} //namespace control
} //namespace uquad
