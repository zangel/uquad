#ifndef UQUAD_CONTROL_BAROMETER_H
#define UQUAD_CONTROL_BAROMETER_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class Barometer
        : public virtual Block
    {
    public:
        typedef float Pressure;
        typedef float Altitude;
        typedef float Temperature;
        
        static std::string const& pressureName();
        static std::string const& relativeAltitudeName();
        static std::string const& temperatureName();
        
        Barometer();
        ~Barometer();
        
    protected:
        DefaultOutputSignal<Pressure> m_Pressure;
        DefaultOutputSignal<Altitude> m_RelativeAltitude;
        DefaultOutputSignal<Temperature> m_Temperature;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BAROMETER_H
