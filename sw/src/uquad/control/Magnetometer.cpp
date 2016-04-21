#include "Magnetometer.h"

namespace uquad
{
namespace control
{
    std::string const& Magnetometer::magneticFieldName()
    {
        static std::string _name = "magneticField";
        return _name;
    }
    
    Magnetometer::Magnetometer()
        : Block()
        , m_MagneticField(magneticFieldName(), MagneticField::Zero())
    {
        intrusive_ptr_add_ref(&m_MagneticField); addOutputPort(&m_MagneticField);
    }
    
    Magnetometer::~Magnetometer()
    {
    }
    
} //namespace control
} //namespace uquad
