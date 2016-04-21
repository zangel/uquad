#ifndef UQUAD_CONTROL_ATTITUDE_CONTROL_H
#define UQUAD_CONTROL_ATTITUDE_CONTROL_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class AttitudeControl
        : public virtual Block
    {
    public:
        typedef float DT;
        typedef Quaternionf Attitude;
        typedef Vec3f RotationRate;
        
        
        static std::string const& dTName();
        static std::string const& attitudeName();
        static std::string const& attitudeSetpointName();
        static std::string const& rotationRateName();
        static std::string const& rotationRateSetpointName();
        
        AttitudeControl();
        ~AttitudeControl();
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        
    protected:
        DefaultInputSignal<DT> m_DT;
        DefaultInputSignal<Attitude> m_Attitude;
        DefaultInputSignal<Attitude> m_AttitudeSetpoint;
        DefaultInputSignal<RotationRate> m_RotationRate;
        DefaultOutputSignal<RotationRate> m_RotationRateSetpoint;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_ATTITUDE_CONTROL_H
