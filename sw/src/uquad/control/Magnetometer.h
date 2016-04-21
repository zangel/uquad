#ifndef UQUAD_CONTROL_MAGNETOMETER_H
#define UQUAD_CONTROL_MAGNETOMETER_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class Magnetometer
        : public virtual Block
    {
    public:
        typedef Vec3f MagneticField;
        
        static std::string const& magneticFieldName();
        
        Magnetometer();
        ~Magnetometer();
        
    protected:
        DefaultOutputSignal<MagneticField> m_MagneticField;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_MAGNETOMETER_H
