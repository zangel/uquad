#ifndef UQUAD_CONTROL_BLOCKS_BAROMETER_H
#define UQUAD_CONTROL_BLOCKS_BAROMETER_H

#include "../Config.h"
#include "../Block.h"
#include "../Signal.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class Barometer
        : public virtual Block
    {
    public:
        typedef float Pressure;
        typedef float Altitude;
        typedef float Temperature;
        
        Barometer();
        ~Barometer();
        
        intrusive_ptr<OutputSignal> outputSignalPressure() const;
        intrusive_ptr<OutputSignal> outputSignalAltitude() const;
        intrusive_ptr<OutputSignal> outputSignalTemperature() const;
        
    protected:
        mutable DefaultOutputSignal<Pressure> m_OSPressure;
        mutable DefaultOutputSignal<Altitude> m_OSAltitude;
        mutable DefaultOutputSignal<Temperature> m_OSTemperature;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_BAROMETER_H
