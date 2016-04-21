#ifndef UQUAD_CONTROL_QUAD_MOTORS_THRUST_H
#define UQUAD_CONTROL_QUAD_MOTORS_THRUST_H

#include "Config.h"
#include "MotorsThrust.h"

namespace uquad
{
namespace control
{
    class QuadMotorsThrust
        : public MotorsThrust
    {
    public:
        typedef float Thrust;
        typedef Vec3f RotationRate;
        typedef Vec4f Motors;
        
        QuadMotorsThrust();
        ~QuadMotorsThrust();
        
        Vec4f getMotors() const;
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        DefaultInputSignal<Thrust> m_ThrustSetpoint;
        DefaultInputSignal<RotationRate> m_RotationRateSetpoint;
        DefaultOutputSignal<Motors> m_Motors;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_QUAD_MOTORS_THRUST_H
