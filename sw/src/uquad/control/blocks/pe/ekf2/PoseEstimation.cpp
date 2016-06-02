#include "PoseEstimation.h"

namespace uquad
{
namespace control
{
namespace blocks
{
namespace pe
{
namespace ekf2
{

    PoseEstimation::Registry::Registry()
        : control::blocks::PoseEstimation::Registry("ekf2")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::blocks::PoseEstimation()
        , m_bOnGround(true)
        , m_bArmed(true)
        , m_EKF()
        , m_LastRatesInitialized(false)
        , m_LastVelocityRate()
        , m_LastRotationRate()
    {
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
        if(system::error_code pee = control::blocks::PoseEstimation::prepare(yctx))
            return pee;
        
        if( !m_ISDT.isConnected() ||
            !m_ISTime.isConnected() ||
            !m_ISVelocityRate.isConnected() ||
            !m_ISRotationRate.isConnected() ||
            !m_ISMagneticField.isConnected() ||
            !m_ISAltitude.isConnected())
        {
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_LastRatesInitialized = false;
        m_LastVelocityRate.setZero();
        m_LastRotationRate.setZero();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void PoseEstimation::unprepare(asio::yield_context yctx)
    {
        control::blocks::PoseEstimation::unprepare(yctx);
    }
    
    #define EKF2_USE_VR 1
    #define EKF2_USE_RR 1
    #define EKF2_USE_MF 1
    #define EKF2_USE_BA 1
    
    system::error_code PoseEstimation::update(asio::yield_context yctx)
    {
        #if EKF2_USE_VR
        Vec3f velocityRate = -9.80665f * m_ISVelocityRate.m_Value;
        #else
        Vec3f velocityRate = Vec3f(
            0.0f,
            0.0f,
            -9.80665f
        );
        #endif
        
        #if EKF2_USE_RR
        Vec3f rotationRate = m_ISRotationRate.m_Value;
        #else
        Vec3f rotationRate = Vec3f::Zero();
        #endif
        
        #if EKF2_USE_MF
        Vec3f magneticField = -m_ISMagneticField.m_Value;
        #else
        Vec3f magneticField = Vec3f::Zero();
        #endif
        
        #if EKF2_USE_BA
        float baroHeight = m_ISAltitude.m_Value;
        #else
        float baroHeight = 0.0f;
        #endif
        
        
        if(!m_LastRatesInitialized)
        {
            m_LastRatesInitialized = true;
            m_LastVelocityRate = velocityRate;
            m_LastRotationRate = rotationRate;
        }
        
        uint64_t const absTus = std::lround(1.0e+6f*m_ISTime.m_Value);
        uint64_t const dTus = std::lround(1.0e+6f*m_ISDT.m_Value);
        
        
        Vec3f imuVelRateData = 0.5f*(velocityRate + m_LastVelocityRate)*m_ISDT.m_Value;
        Vec3f imuRotRateData = 0.5f*(rotationRate + m_LastRotationRate)*m_ISDT.m_Value;
        Vec3f magData = magneticField;
        float baroData = baroHeight;
        
        m_EKF.set_in_air_status(!m_bOnGround);
        m_EKF.set_arm_status(m_bArmed);
        m_EKF.setIMUData(absTus, dTus, dTus, imuRotRateData.data(), imuVelRateData.data());
        m_EKF.setMagData(absTus, magData.data());
        m_EKF.setBaroData(absTus, &baroData);
        
        
        m_EKF.update();
        
        float estData[4];

        
        
        m_EKF.copy_quaternion(estData);
        m_OSAttitude.m_Value = Quaternionf(estData[0], estData[1], estData[2], estData[3]);
        
        m_EKF.copy_position(estData);
        m_OSPosition.m_Value = -Vec3f(estData[0], estData[1], estData[2]);
        
        m_EKF.copy_velocity(estData);
        m_OSVelocity.m_Value = -Vec3f(estData[0], estData[1], estData[2]);

        
        m_EKF.get_heading_innov(estData);
        m_OSEarthMagneticField.m_Value = Vec3f(estData[0], estData[0], estData[0]);
        
        m_LastVelocityRate = velocityRate;
        m_LastRotationRate = rotationRate;
        
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ekf2
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad
