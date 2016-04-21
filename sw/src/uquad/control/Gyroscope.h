#ifndef UQUAD_CONTROL_GYROSCOPE_H
#define UQUAD_CONTROL_GYROSCOPE_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class Gyroscope
        : public virtual Block
    {
    public:
        typedef Vec3f RotationRate;
        
        static std::string const& rotationRateName();
        
        Gyroscope();
        ~Gyroscope();
        
    protected:
        
    protected:
        DefaultOutputSignal<RotationRate> m_RotationRate;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_GYROSCOPE_H
