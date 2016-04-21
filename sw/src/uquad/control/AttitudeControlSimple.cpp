#include "AttitudeControlSimple.h"

namespace uquad
{
namespace control
{
    AttitudeControlSimple::AttitudeControlSimple()
        : AttitudeControl()
        , m_AttitudeP(1.0f)
        , m_RotationRateP(0.49f, 0.49f, 0.0f)
        , m_RotationRateI(0.0f, 0.0f, 0.0f)
        , m_RotationRateD(0.01f, 0.01f, 0.0f)
        , m_RotationRatePrev(Vec3f::Zero())
        , m_RotationRateSetpointPrev(Vec3f::Zero())
    {
    }
    
    AttitudeControlSimple::~AttitudeControlSimple()
    {
    }
    
    bool AttitudeControlSimple::isValid() const
    {
        return true;
    }
    
    void AttitudeControlSimple::setAttitudeControlP(float p)
    {
        m_AttitudeP = p;
    }
    
    void AttitudeControlSimple::setRotationRateControlPID(Vec3f p, Vec3f i, Vec3f d)
    {
        m_RotationRateP = p;
        m_RotationRateI = i;
        m_RotationRateD = d;
    }
    
    system::error_code AttitudeControlSimple::prepare(asio::yield_context yctx)
    {
        if(system::error_code ae = AttitudeControl::prepare(yctx))
            return ae;
        
        m_RotationRatePrev = Vec3f::Zero();
        m_RotationRateSetpointPrev = Vec3f::Zero();
        
        m_RotationRateIError = Vec3f::Zero();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code AttitudeControlSimple::update(asio::yield_context yctx)
    {
        static once_flag initSetpoint;
        
        call_once(initSetpoint, [this]() { m_AttitudeSetpoint.m_Value = m_Attitude.m_Value; } );
        
        #if 0
        Mat3x3f R = m_Attitude.m_Value.toRotationMatrix();
        Mat3x3f R_sp = m_AttitudeSetpoint.m_Value.toRotationMatrix();
        #else
        Mat3x3f R = m_Attitude.m_Value.toRotationMatrix();
        #if 0
        Mat3x3f R_sp = Quaternionf(AngleAxisf(M_PI_4, Vec3f::UnitX())).toRotationMatrix();
        #else
        Mat3x3f R_sp = m_AttitudeSetpoint.m_Value.toRotationMatrix();
        #endif
        #endif
        
        float dT = std::min(std::max(m_DT.m_Value, 0.002f), 0.02f);
        
        Vec3f R_z(R(0, 2), R(1, 2), R(2, 2));
        Vec3f R_sp_z(R_sp(0, 2), R_sp(1, 2), R_sp(2, 2));
        
        Vec3f e_R = R.transpose() * (R_z.cross(R_sp_z));
        
        float e_R_z_sin = e_R.norm();
        float e_R_z_cos = R_z.dot(R_sp_z);

        float yaw_w = R_sp(2, 2) * R_sp(2, 2);
        
        Mat3x3f R_rp;
        
        if(e_R_z_sin > 0.0f)
        {
            float e_R_z_angle = std::atan2(e_R_z_sin, e_R_z_cos);
            Vec3f e_R_z_axis = e_R / e_R_z_sin;
            
            e_R = e_R_z_angle * e_R_z_axis;
            
            Mat3x3f e_R_cp = Mat3x3f::Zero();
            e_R_cp(0, 1) = -e_R_z_axis(2);
            e_R_cp(0, 2) = e_R_z_axis(1);
            e_R_cp(1, 0) = e_R_z_axis(2);
            e_R_cp(1, 2) = -e_R_z_axis(0);
            e_R_cp(2, 0) = -e_R_z_axis(1);
            e_R_cp(2, 1) = e_R_z_axis(0);

            R_rp = R * (Mat3x3f::Identity() + e_R_cp * e_R_z_sin + e_R_cp * e_R_cp * (1.0f - e_R_z_cos));
        }
        else
        {
            R_rp = R;
        }
        
        Vec3f R_sp_x(R_sp(0, 0), R_sp(1, 0), R_sp(2, 0));
        Vec3f R_rp_x(R_rp(0, 0), R_rp(1, 0), R_rp(2, 0));
        e_R(2) = std::atan2(R_rp_x.cross(R_sp_x).dot(R_sp_z), R_rp_x.dot(R_sp_x)) * yaw_w;
        
        if(e_R_z_cos < 0.0f)
        {
            Quaternionf q(R.transpose() * R_sp);
            Vec3f e_R_d = q.vec();
            e_R_d.normalize();
            e_R_d *= 2.0f * std::atan2(e_R_d.norm(), q.w());
            
            float direct_w = e_R_z_cos * e_R_z_cos * yaw_w;
            e_R = e_R * (1.0f - direct_w) + e_R_d * direct_w;
        }
        
        Vec3f rotationRateSetpoint = m_AttitudeP * e_R;
        Vec3f e_RR = rotationRateSetpoint - m_RotationRate.m_Value;
        
        
        Vec3f rotationRateControl =
            m_RotationRateP.cwiseProduct(e_RR) +
            m_RotationRateD.cwiseProduct(m_RotationRatePrev - m_RotationRate.m_Value) / dT +
            m_RotationRateIError;
        
        m_RotationRatePrev = m_RotationRate.m_Value;
        m_RotationRateSetpointPrev = rotationRateSetpoint;
        
        m_RotationRatePrev = m_RotationRate.m_Value;
        m_RotationRateSetpointPrev = rotationRateSetpoint;
        
        m_RotationRateSetpoint.m_Value = rotationRateControl;
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace control
} //namespace uquad
