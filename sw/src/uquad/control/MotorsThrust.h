#ifndef UQUAD_CONTROL_MOTORS_THRUST_H
#define UQUAD_CONTROL_MOTORS_THRUST_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class MotorsThrust
        : public virtual Block
    {
    public:
        
        static std::string const& thrustSetpointName();
        static std::string const& rotationRateSetpointName();
        static std::string const& motorsName();
        
        MotorsThrust();
        ~MotorsThrust();
        
    protected:
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_MOTORS_THRUST_H
