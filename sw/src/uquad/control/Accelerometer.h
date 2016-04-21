#ifndef UQUAD_CONTROL_ACCELEROMETER_H
#define UQUAD_CONTROL_ACCELEROMETER_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class Accelerometer
        : public virtual Block
    {
    public:
        typedef Vec3f VelocityRate;
        
        static std::string const& velocityRateName();
        
        Accelerometer();
        ~Accelerometer();
        
    protected:
        
    protected:
        DefaultOutputSignal<VelocityRate> m_VelocityRate;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_ACCELEROMETER_H
