#include "Accelerometer.h"

namespace uquad
{
namespace control
{
    std::string const& Accelerometer::velocityRateName()
    {
        static std::string _name = "velocityRate";
        return _name;
    }
    
    Accelerometer::Accelerometer()
        : Block()
        , m_VelocityRate(velocityRateName(), VelocityRate::Zero())
    {
        intrusive_ptr_add_ref(&m_VelocityRate); addOutputPort(&m_VelocityRate);
    }
    
    Accelerometer::~Accelerometer()
    {
    }
    
} //namespace control
} //namespace uquad
