#ifndef UQUAD_CONTROL_POSITION_CONTROL_SIMPLE_H
#define UQUAD_CONTROL_POSITION_CONTROL_SIMPLE_H

#include "Config.h"
#include "PositionControl.h"
#include "Filters.h"

namespace uquad
{
namespace control
{
    class PositionControlSimple
        : public PositionControl
    {
    public:
        PositionControlSimple();
        ~PositionControlSimple();
        
        bool isValid() const;

    protected:
        system::error_code prepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
    private:
        void resetPositionSP();
        void resetAltitudeSP();
        void resetAttitudeSP();
        
        void directionControl();
        void thrustControl();
        void attitudeControl();
        
        
        
    private:
        Vec3f m_ParamPositionP;
        Vec3f m_ParamVelocityP;
        Vec3f m_ParamVelocityI;
        Vec3f m_ParamVelocityD;
        Vec3f m_ParamCruiseVelocity;
        float m_ParamHoldXYThreshold;
        float m_ParamHoldXYMax;
        float m_ParamHoldZMax;
        float m_ParamThrustMin;
        float m_ParamThrustMax;
        float m_ParamYawP;
        float m_ParamYawRateMax;
        
        LowPassFilter<Vec3f> m_VelocityLP;
        
        Mat3x3f m_AttitudeMat;
        Vec3f m_AttitudeEuler;
        
        
        Vec3f m_PositionSP;
        Vec3f m_VelocitySP;
        
        Mat3x3f m_AttitudeSPMat;
        Vec3f m_AttitudeSPEuler;

        Vec3f m_ThrustSP;
        float m_ThrustAbs;
        Vec3f m_ThrustInt;
        
        Vec3f m_VelocityErrorD;
        Vec3f m_VelocityError;
        
        bool m_bResetYawSP;
        bool m_bResetPositionSP;
        bool m_bResetAltitudeSP;
        bool m_bResetThrustXYInt;
        bool m_bResetThrustZInt;
        
        bool m_bPositionHoldEngaged;
        bool m_bAltitudeHoldEngaged;
        
        bool m_bRunPositionControl;
        bool m_bRunAltitudeControl;
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_POSITION_CONTROL_SIMPLE_H
