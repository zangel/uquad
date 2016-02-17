#include "PoseEstimation.h"
#include "../SystemLibrary.h"
#include "../../base/Error.h"

namespace uquad
{
namespace control
{
namespace ekf2
{
    PoseEstimation::Registry::Registry()
        : control::PoseEstimation::Registry("ekf2")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::PoseEstimation()
        , m_bPrepared(false)
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
        
        m_IMUStartTime = base::TimePoint::min();
        m_IMULastTime = base::TimePoint::min();
        
        m_LastVelRate.setZero();
        m_LastAngRate.setZero();
        
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
        if(m_IMUStartTime == base::TimePoint::min())
        {
            m_IMUStartTime = usd.time;
            m_IMULastTime = usd.time;
            m_LastVelRate = usd.velocityRate;
            m_LastAngRate = usd.rotationRate;
            return base::makeErrorCode(base::kENoError);
        }
        
        base::TimeDuration const absT = usd.time - m_IMUStartTime;
        base::TimeDuration const dT = usd.time - m_IMULastTime;
        
        uint64_t const absTus = chrono::duration_cast<chrono::microseconds>(absT).count();
        uint64_t const dTus = chrono::duration_cast<chrono::microseconds>(dT).count();
        float const dTs = dTus*1.0e-6;
        
        Vec3f dVel = 0.5f*(usd.velocityRate + m_LastVelRate)*dTs;
        Vec3f dAng = 0.5f*(usd.rotationRate + m_LastAngRate)*dTs;
        Vec3f magneticField = usd.magneticField;
        float baroAltitude = usd.baroPressure;
        
        #if 0
        dVel(0) = dVel(1) = 0.0f;
        dAng.setZero();
        magneticField(0) = magneticField(1) = 0.0f;
        magneticField(2) = 40;
        baroAltitude = 0.0f;
        #endif
        
        m_EKF.setIMUData(absTus, dTus, dTus, dVel.data(), dAng.data());
        m_EKF.setMagData(absTus, magneticField.data());
        m_EKF.setBaroData(absTus, &baroAltitude);
        
        m_EKF.update();
        
        attitude = m_EKF.getAttitude();
        position = m_EKF.getPosition();
        velocity = m_EKF.getVelocity();
        
        earthMagneticField = m_EKF.getMagI();
        
        m_IMULastTime = usd.time;
        m_LastVelRate = usd.velocityRate;
        m_LastAngRate = usd.rotationRate;
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ekf2
} //namespace control
} //namespace uquad

UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf2::PoseEstimation, uquad::control::SystemLibrary)