#include "PoseEstimation.h"
#include "../SystemLibrary.h"
#include "../../base/Error.h"
#include "../../math/Utils.h"


namespace uquad
{
namespace control
{
namespace madgwick
{
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
        , m_bPrepared(false)
        , m_IMUFilter()
        , m_IMUFilterInitialized(false)
        , m_IMUStartTime()
        , m_IMULastTime()
    {
    }
    
    PoseEstimation::~PoseEstimation()
    {
    }
    
    bool PoseEstimation::isValid() const
    {
        return true;
    }
        
    system::error_code PoseEstimation::prepare()
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_IMUFilter.setAlgorithmGain(1.0f);
        m_IMUFilter.setDriftBiasGain(0.0f);
        
        m_IMUFilterInitialized = false;
        m_IMUStartTime = base::TimePoint::min();
        m_IMULastTime = base::TimePoint::min();
        
        m_MagneticBias.setZero();
        
        m_bPrepared = true;
        return base::makeErrorCode(base::kENoError);
    }
    
    bool PoseEstimation::isPrepared() const
    {
        return m_bPrepared;
    }
    
    void PoseEstimation::unprepare()
    {
        if(!m_bPrepared)
            return;
        
        m_bPrepared = false;
    }
    
    system::error_code PoseEstimation::processUQuadSensorsData(hal::UQuadSensorsData const &usd)
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kEInvalidState);
        
        if(m_IMUStartTime == base::TimePoint::min())
        {
            m_IMUStartTime = usd.time;
            m_IMULastTime = usd.time;
            return base::makeErrorCode(base::kENoError);
        }
        
        base::TimeDuration const dT = usd.time - m_IMULastTime;
        #if 0
        float const dTs = chrono::duration_cast<chrono::microseconds>(dT).count()*1.0e-6f;
        #else
        float const dTs = 1.0f/50.0f;
        #endif
        
        Vec3f mag = usd.magneticField - m_MagneticBias;
        
        if(!m_IMUFilterInitialized)
        {
            if(!std::isfinite(usd.magneticField.norm()))
                return base::makeErrorCode(base::kENoError);
            
            Quaternionf initQ = computeOrientation(usd.velocityRate, mag);
            m_IMUFilter.setOrientation(initQ.w(), initQ.x(), initQ.y(), initQ.z());
            
            m_IMUFilterInitialized = true;
        }
        
        m_IMUFilter.madgwickAHRSupdate(
            usd.rotationRate.x(), usd.rotationRate.y(), usd.rotationRate.z(),
            usd.velocityRate.x(), usd.velocityRate.y(), usd.velocityRate.z(),
            mag.x(), mag.y(), mag.z(),
            dTs
        );
        
        double attQ0, attQ1, attQ2, attQ3;
        m_IMUFilter.getOrientation(attQ0, attQ1, attQ2, attQ3);
        
        attitude = Quaternionf(attQ0, attQ1, attQ2, attQ3);
        
        m_IMULastTime = usd.time;
        return base::makeErrorCode(base::kENoError);
    }
    
    Quaternionf PoseEstimation::computeOrientation(Vec3f const &vr, Vec3f const &mf)
    {
        // initialize roll/pitch orientation from acc. vector.
        float sign = std::copysign(1.0f, vr.z());
        float roll = std::atan2(vr.y(), sign * std::sqrt(vr.x()*vr.x() + vr.z()*vr.z()));
        float pitch = -std::atan2(vr.x(), std::sqrt(vr.y()*vr.y() + vr.z()*vr.z()));
        float cos_roll = std::cos(roll);
        float sin_roll = std::sin(roll);
        float cos_pitch = std::cos(pitch);
        float sin_pitch = std::sin(pitch);

        // initialize yaw orientation from magnetometer data.
        /***  From: http://cache.freescale.com/files/sensors/doc/app_note/AN4248.pdf (equation 22). ***/
        float head_x = mf.x() * cos_pitch + mf.y() * sin_pitch * sin_roll + mf.z() * sin_pitch * cos_roll;
        float head_y = mf.y() * cos_roll - mf.z() * sin_roll;
        float yaw = std::atan2(-head_y, head_x);
        
        return math::quatFromAxisAngle(Vec3f(roll, pitch, yaw));
    }

} //namespace ikf
} //namespace control
} //namespace uquad

UQUAD_BASE_REGISTER_OBJECT(uquad::control::madgwick::PoseEstimation, uquad::control::SystemLibrary)