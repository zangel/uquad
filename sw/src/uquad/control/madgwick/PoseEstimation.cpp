#include "PoseEstimation.h"
#include "../../base/Error.h"
#include "../../math/Utils.h"

#define GTOMSEC2 9.80665
#define FPIOVER180 0.01745329251994F
#define USE_MAGNETOMETER 0


namespace uquad
{
namespace control
{
namespace madgwick
{
    namespace
    {
        Quaternionf computeOrientation(Vec3f const &vr, Vec3f const &mf)
        {
            // initialize roll/pitch orientation from acc. vector.
            float sign = ::copysignf(1.0f, vr.z());
            float roll = ::atan2f(vr.y(), sign * std::sqrt(vr.x()*vr.x() + vr.z()*vr.z()));
            float pitch = -::atan2f(vr.x(), std::sqrt(vr.y()*vr.y() + vr.z()*vr.z()));
            float cos_roll = ::cosf(roll);
            float sin_roll = ::sinf(roll);
            float cos_pitch = ::cosf(pitch);
            float sin_pitch = ::sinf(pitch);

            // initialize yaw orientation from magnetometer data.
            /***  From: http://cache.freescale.com/files/sensors/doc/app_note/AN4248.pdf (equation 22). ***/
            float head_x = mf.x() * cos_pitch + mf.y() * sin_pitch * sin_roll + mf.z() * sin_pitch * cos_roll;
            float head_y = mf.y() * cos_roll - mf.z() * sin_roll;
            float yaw = std::atan2(-head_y, head_x);
            
            return math::quatFromAxisAngle(Vec3f(roll, pitch, yaw));
        }
    
    }
    PoseEstimation::Registry::Registry()
        : control::PoseEstimation::Registry("madgwick")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::PoseEstimation()
        , m_DT(dTName())
        , m_VelocityRate(velocityRateName())
        , m_RotationRate(rotationRateName())
        , m_MagneticField(magneticFieldName())
        , m_Attitude(attitudeName())
        , m_IMUFilter()
    {
        intrusive_ptr_add_ref(&m_DT); addInputPort(&m_DT);
        intrusive_ptr_add_ref(&m_VelocityRate); addInputPort(&m_VelocityRate);
        intrusive_ptr_add_ref(&m_RotationRate); addInputPort(&m_RotationRate);
        intrusive_ptr_add_ref(&m_MagneticField); addInputPort(&m_MagneticField);
        
        intrusive_ptr_add_ref(&m_Attitude); addOutputPort(&m_Attitude);
    }
    
    PoseEstimation::~PoseEstimation()
    {
    }
    
    bool PoseEstimation::isValid() const
    {
        return true;
    }
        
    system::error_code PoseEstimation::prepare(asio::yield_context yctx)
    {
        if(system::error_code pee = control::PoseEstimation::prepare(yctx))
            return pee;
        
        if( !m_DT.sourceConnection() ||
            !m_VelocityRate.sourceConnection() ||
            !m_RotationRate.sourceConnection() ||
            !m_MagneticField.sourceConnection())
        {
            return base::makeErrorCode(base::kEInvalidState);
        }

        
        m_IMUFilter.setAlgorithmGain(0.25f);
        m_IMUFilter.setDriftBiasGain(0.0f);
        m_IMUFilterInitialized = false;
        
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void PoseEstimation::unprepare(asio::yield_context yctx)
    {
        control::PoseEstimation::unprepare(yctx);
    }

    
    system::error_code PoseEstimation::update(asio::yield_context yctx)
    {
        #if USE_MAGNETOMETER
        Vec3f mag = m_MagneticField.m_Value;
        
        if(!m_IMUFilterInitialized)
        {
            if(!std::isfinite(m_MagneticField.m_Value.norm()))
                return base::makeErrorCode(base::kENoError);
            
            Quaternionf initQ = computeOrientation(m_VelocityRate.m_Value, mag);
            m_IMUFilter.setOrientation(initQ.w(), initQ.x(), initQ.y(), initQ.z());
            
            m_IMUFilterInitialized = true;
        }
        
        m_IMUFilter.madgwickAHRSupdate(
            m_RotationRate.m_Value.x(), m_RotationRate.m_Value.y(), m_RotationRate.m_Value.z(),
            m_VelocityRate.m_Value.x(), m_VelocityRate.m_Value.y(), m_VelocityRate.m_Value.z(),
            mag.x(), mag.y(), mag.z(),
            m_DT.m_Value
        );
        #else
        m_IMUFilterInitialized = true;
        m_IMUFilter.madgwickAHRSupdateIMU(
            m_RotationRate.m_Value.x(), m_RotationRate.m_Value.y(), m_RotationRate.m_Value.z(),
            m_VelocityRate.m_Value.x(), m_VelocityRate.m_Value.y(), m_VelocityRate.m_Value.z(),
            m_DT.m_Value
        );
        #endif
        
        double attQ0, attQ1, attQ2, attQ3;
        m_IMUFilter.getOrientation(attQ0, attQ1, attQ2, attQ3);
        
        m_Attitude.m_Value = Quaternionf(attQ0, attQ1, attQ2, attQ3);
        
        return base::makeErrorCode(base::kENoError);
    }

    

} //namespace madgwick
} //namespace control
} //namespace uquad
