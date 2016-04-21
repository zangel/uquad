#include "MotorsThrust.h"

namespace uquad
{
namespace control
{
    std::string const& MotorsThrust::thrustSetpointName()
    {
        static std::string _name = "thrustSetpoint";
        return _name;
    }
    
    std::string const& MotorsThrust::rotationRateSetpointName()
    {
        static std::string _name = "rotationRateSetpoint";
        return _name;
    }
    
    std::string const& MotorsThrust::motorsName()
    {
        static std::string _name = "motors";
        return _name;
    }
    
    
    MotorsThrust::MotorsThrust()
        : Block()
    {
    }
    
    MotorsThrust::~MotorsThrust()
    {
    }
    

} //namespace control
} //namespace uquad