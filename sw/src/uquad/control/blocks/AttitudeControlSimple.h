#ifndef UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_SIMPLE_H
#define UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_SIMPLE_H

#include "../Config.h"
#include "AttitudeControl.h"

namespace uquad
{
namespace control
{
namespace blocks
{
    class AttitudeControlSimple
        : public AttitudeControl
    
    {
    public:
        AttitudeControlSimple();
        ~AttitudeControlSimple();
        
        bool isValid() const;

        void setAttitudeControlP(Vec3f p);
        void setRotationRateControlPID(Vec3f p, Vec3f i, Vec3f d);
        
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
    protected:
        Vec3f m_AttitudeP;
        Vec3f m_RotationRateP;
        Vec3f m_RotationRateI;
        Vec3f m_RotationRateD;

        Vec3f m_RotationRateMeasuredPrev;
        Vec3f m_RotationRateSetpointPrev;
        
        Vec3f m_RotationRateIError;
    };

} //namespace blocks
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_BLOCKS_ATTITUDE_CONTROL_SIMPLE_H
