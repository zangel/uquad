#include "PositionControlSimple.h"

#define TILT_COS_MAX	0.7f
#define SIGMA			0.000001f

namespace uquad
{
namespace control
{
    PositionControlSimple::PositionControlSimple()
        : PositionControl()
        , m_ParamPositionP(1.25f, 1.25f, 1.0f)
        , m_ParamVelocityP(0.08f, 0.08f, 0.2f)
        , m_ParamVelocityI(0.02f, 0.02f, 0.02f)
        , m_ParamVelocityD(0.01f, 0.01f, 0.0f)
        , m_ParamCruiseVelocity(5.0f, 5.0f, 1.0f)
        , m_ParamHoldXYThreshold(0.1f)
        , m_ParamHoldXYMax(0.8f)
        , m_ParamHoldZMax(0.6f)
        , m_ParamThrustMin(0.1f)
        , m_ParamThrustMax(1.0f)
        , m_ParamYawP(2.8f)
        , m_ParamYawRateMax(60.0f)
        , m_VelocityLP(5.0f, Vec3f::Zero())
        , m_AttitudeMat(Mat3x3f::Identity())
        , m_AttitudeEuler(Vec3f::Zero())
        , m_PositionSP(Vec3f::Zero())
        , m_VelocitySP(Vec3f::Zero())
        , m_AttitudeSPMat(Mat3x3f::Identity())
        , m_AttitudeSPEuler(Vec3f::Zero())
        , m_ThrustSP(Vec3f::Zero())
        , m_ThrustAbs(0.0f)
        , m_ThrustInt(Vec3f::Zero())
        , m_VelocityErrorD(Vec3f::Zero())
        , m_VelocityError(Vec3f::Zero())
        , m_bResetYawSP(false)
        , m_bResetPositionSP(false)
        , m_bResetAltitudeSP(false)
        , m_bResetThrustXYInt(false)
        , m_bResetThrustZInt(false)
        , m_bPositionHoldEngaged(false)
        , m_bAltitudeHoldEngaged(false)
        , m_bRunPositionControl(false)
        , m_bRunAltitudeControl(false)
    {
    }
    
    PositionControlSimple::~PositionControlSimple()
    {
    }
    
    bool PositionControlSimple::isValid() const
    {
        return true;
    }
    
    system::error_code PositionControlSimple::prepare(asio::yield_context yctx)
    {
        if(system::error_code ae = PositionControl::prepare(yctx))
            return ae;
        
        m_VelocityLP.reset();
        
        m_AttitudeMat.setIdentity();
        m_AttitudeEuler.setZero();
        m_PositionSP.setZero();
        m_VelocitySP.setZero();
        m_AttitudeSPMat.setIdentity();
        m_AttitudeSPEuler.setZero();
        m_AttitudeSPMat.setIdentity();
        m_ThrustSP.setZero();
        m_ThrustAbs = 0.0f;
        m_ThrustInt.setZero();
        m_VelocityErrorD.setZero();
        m_VelocityError.setZero();
        m_bResetYawSP = true;
        m_bResetPositionSP = true;
        m_bResetAltitudeSP = true;
        m_bResetThrustXYInt = true;
        m_bResetThrustZInt = true;
        m_bPositionHoldEngaged = false;
        m_bAltitudeHoldEngaged = false;
        m_bRunPositionControl = false;
        m_bRunAltitudeControl = false;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PositionControlSimple::update(asio::yield_context yctx)
    {
        m_VelocityLP.update(-m_Velocity.m_Value, m_DT.m_Value);
        m_AttitudeMat = m_Attitude.m_Value.toRotationMatrix();
        m_AttitudeEuler = m_AttitudeMat.eulerAngles(0, 1, 2);
        
        m_VelocityErrorD = m_VelocityLP.state();
        
        m_bRunPositionControl = true;
        m_bRunAltitudeControl = true;
        
        directionControl();
        thrustControl();
        attitudeControl();
        
        m_ThrustSetpoint.m_Value = m_ThrustAbs;
        m_AttitudeSetpoint.m_Value = Quaternionf(m_AttitudeSPMat);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void PositionControlSimple::resetPositionSP()
    {
        if(m_bResetPositionSP)
        {
            m_bResetPositionSP = false;
            m_PositionSP(0) = m_Position.m_Value(0);
            m_PositionSP(1) = m_Position.m_Value(1);
        }
    }
    
    void PositionControlSimple::resetAltitudeSP()
    {
        if(m_bResetAltitudeSP)
        {
            m_bResetAltitudeSP = false;
            m_PositionSP(2) = m_Position.m_Value(2);
        }
    }
    
    void PositionControlSimple::resetAttitudeSP()
    {
        if(m_bResetYawSP)
        {
            m_bResetYawSP = false;
            m_AttitudeSPEuler(2) = m_AttitudeEuler(2);
        }
    }
    
    void PositionControlSimple::directionControl()
    {
        Vec3f reqVelSP = Vec3f(
            m_DirectionControl.m_Value.x(),
            m_DirectionControl.m_Value.y(),
            #if 0
            m_ThrustControl.m_Value - 0.5f
            #else
            m_ThrustControl.m_Value
            #endif
        );
        
        resetAltitudeSP();
        resetPositionSP();
        resetAttitudeSP();
        
        float const reqVelSPNorm = reqVelSP.norm();
        
        if(reqVelSPNorm > 1.0f)
        {
            reqVelSP /= reqVelSPNorm;
        }
        
        Mat3x3f rotYawSP;
        rotYawSP = AngleAxisf(m_AttitudeSPEuler(2), Vec3f::UnitZ());
        Vec3f reqVelSPScaled = rotYawSP * reqVelSP.cwiseProduct(m_ParamCruiseVelocity);
        
        
        if( std::abs(reqVelSP(0)) < m_ParamHoldXYThreshold &&
            std::abs(reqVelSP(1)) < m_ParamHoldXYThreshold )
        {
			if(!m_bPositionHoldEngaged)
            {
                if( m_ParamHoldXYMax < std::numeric_limits<float>::epsilon() ||
                    (
                        std::abs(m_Velocity.m_Value(0)) < m_ParamHoldXYMax &&
                        std::abs(m_Velocity.m_Value(0)) < m_ParamHoldXYMax
                    ) )
                {
					m_bPositionHoldEngaged = true;

				}
                else
                {
					m_bPositionHoldEngaged = false;
				}
			}
		}
        else
        {
			m_bPositionHoldEngaged = false;
		}

		if(!m_bPositionHoldEngaged)
        {
            m_PositionSP(0) = m_Position.m_Value(0);
            m_PositionSP(1) = m_Position.m_Value(1);
            
			m_bRunPositionControl = false;
            
            m_VelocitySP(0) = reqVelSPScaled(0);
            m_VelocitySP(1) = reqVelSPScaled(1);
		}
        
		if(std::abs(reqVelSP(2)) < std::numeric_limits<float>::epsilon())
        {
			if(!m_bAltitudeHoldEngaged)
            {
				if( m_ParamHoldZMax < std::numeric_limits<float>::epsilon() ||
                    std::abs(m_Velocity.m_Value(2)) < m_ParamHoldZMax )
                {
					m_bAltitudeHoldEngaged = true;
				}
                else
                {
					m_bAltitudeHoldEngaged = false;
				}
			}
		}
        else
        {
			m_bAltitudeHoldEngaged = false;
		}

		if(!m_bAltitudeHoldEngaged)
        {
			m_bRunAltitudeControl = false;
            m_VelocitySP(2) = reqVelSPScaled(2);
            m_PositionSP(2) = m_Position.m_Value(2);
		}
        
        float const yawOffsetMax = m_ParamYawRateMax / m_ParamYawP;
        float const yawSPMoveRate = m_DirectionControl.m_Value(2) * m_ParamYawRateMax;
        float yawOffset = yawSPMoveRate * m_DT.m_Value;
        yawOffset = std::copysign(1.0f, yawOffset) * std::min(yawOffsetMax, std::abs(yawOffset));
        
        m_AttitudeSPEuler(2) += yawOffset;
	}
    
    void PositionControlSimple::thrustControl()
    {
        if(m_bRunPositionControl)
        {
            m_VelocitySP(0) = (m_PositionSP(0) - m_Position.m_Value(0)) * m_ParamPositionP(0);
            m_VelocitySP(1) = (m_PositionSP(1) - m_Position.m_Value(1)) * m_ParamPositionP(1);
        }
        
        if(m_bRunAltitudeControl)
        {
            m_VelocitySP(2) = (m_PositionSP(2) - m_Position.m_Value(2)) * m_ParamPositionP(2);
        }
        
        if(m_bResetThrustZInt)
        {
            m_bResetThrustZInt = false;
            m_ThrustInt(2) = 0.0f;
        }
        
        if(m_bResetThrustXYInt)
        {
            m_bResetThrustXYInt = false;
            m_ThrustInt(0) = 0.0f;
            m_ThrustInt(1) = 0.0f;
        }
        
        m_VelocityError = m_VelocitySP - m_Velocity.m_Value;
        
        m_ThrustSP = m_VelocityError.cwiseProduct(m_ParamVelocityP) + m_VelocityErrorD.cwiseProduct(m_ParamVelocityD)/* + m_ThrustInt*/;
        
        bool thrustSatXY = false;
        bool thrustSatZ = false;
        
        m_ThrustAbs = m_ThrustSP.norm();
        
        if(-m_ThrustSP(2) < m_ParamThrustMin)
        {
            m_ThrustSP(2) = -m_ParamThrustMin;
            thrustSatZ = true;
        }
        
        {
            float attComp;
            
            if(m_AttitudeMat(2, 2) > TILT_COS_MAX)
            {
                attComp = 1.0f / m_AttitudeMat(2, 2);
                
            }
            else
            if(m_AttitudeMat(2, 2) > 0.0f)
            {
                attComp = ((1.0f / TILT_COS_MAX - 1.0f) / TILT_COS_MAX) * m_AttitudeMat(2, 2) + 1.0f;
                thrustSatZ = true;
                
            }
            else
            {
                attComp = 1.0f;
                thrustSatZ = true;
            }
            
            m_ThrustSP(2) *= attComp;
        }
        
        m_ThrustAbs = m_ThrustSP.norm();
        if(m_ThrustAbs > m_ParamThrustMax)
        {
            if(m_ThrustSP(2) < 0.0f)
            {
                float k = m_ParamThrustMax / m_ThrustAbs;
                m_ThrustSP *= k;
                thrustSatXY = true;
                thrustSatZ = true;
            }
            else
            {
                if(m_ThrustSP(2) > m_ParamThrustMax)
                {
                    m_ThrustSP(0) = 0.0f;
                    m_ThrustSP(1) = 0.0f;
					m_ThrustSP(2) = m_ParamThrustMax;
					thrustSatXY = true;
                    thrustSatZ = true;
                }
                else
                {
                    float thrustXYMax = std::sqrt(m_ParamThrustMax * m_ParamThrustMax - m_ThrustSP(2) * m_ThrustSP(2));
                    float thrustXYMaxAbs = Vec2f(m_ThrustSP(0), m_ThrustSP(1)).norm();
                    float k = thrustXYMax / thrustXYMaxAbs;
                    m_ThrustSP(0) *= k;
                    m_ThrustSP(1) *= k;
					thrustSatXY = true;
                }
            }
            m_ThrustAbs = m_ParamThrustMax;
        }
        
        if(!thrustSatZ)
        {
            m_ThrustInt(2) += m_VelocityError(2) * m_ParamVelocityI(2) * m_DT.m_Value;
            if(m_ThrustInt(2) > 0.0f)
            {
                m_ThrustInt(2) = 0.0f;
            }
        }
    }
    
    void PositionControlSimple::attitudeControl()
    {
        Vec3f bodyX, bodyY, bodyZ;
        
        if(m_ThrustAbs > SIGMA)
        {
            bodyZ = m_ThrustSP / m_ThrustAbs;
        }
        else
        {
            bodyZ = Vec3f::UnitZ();
        }
        
        Vec3f yC(-std::sin(m_AttitudeSPEuler(2)), std::cos(m_AttitudeSPEuler(2)), 0.0f);
        
        if(std::abs(bodyZ(2)) > SIGMA)
        {
            bodyX = yC.cross(bodyZ);
            
            if(bodyZ(2) < 0.0f)
            {
                bodyX = -bodyX;
            }
            
            bodyX.normalize();
        }
        else
        {
            bodyX = Vec3f::UnitZ();
        }
        
        bodyY = bodyZ.cross(bodyX);
        
        m_AttitudeSPMat.col(0) = bodyX;
        m_AttitudeSPMat.col(1) = bodyY;
        m_AttitudeSPMat.col(2) = bodyZ;
        
        Vec3f eulerAngles = m_AttitudeSPMat.eulerAngles(0, 1, 2);
        
        m_AttitudeSPEuler(0) = eulerAngles(0);
        m_AttitudeSPEuler(1) = eulerAngles(1);
    }
    
} //namespace control
} //namespace uquad
