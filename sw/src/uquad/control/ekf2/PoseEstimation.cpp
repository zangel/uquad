#include "PoseEstimation.h"
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
        , m_DT(dTName())
        , m_Time(timeName())
        , m_VelocityRate(velocityRateName())
        , m_RotationRate(rotationRateName())
        , m_MagneticField(magneticFieldName())
        , m_RelativeAltitude(relativeAltitudeName())
        , m_Attitude(attitudeName())
        , m_Position(positionName())
        , m_Velocity(velocityName())
        , m_EarthMagneticField(earthMagneticFieldName())
        , m_bOnGround(true)
        , m_bArmed(true)
        , m_EKF()
        , m_LastRatesInitialized(false)
        , m_LastVelocityRate()
        , m_LastRotationRate()
    {
        intrusive_ptr_add_ref(&m_DT); addInputPort(&m_DT);
        intrusive_ptr_add_ref(&m_Time); addInputPort(&m_Time);
        intrusive_ptr_add_ref(&m_VelocityRate); addInputPort(&m_VelocityRate);
        intrusive_ptr_add_ref(&m_RotationRate); addInputPort(&m_RotationRate);
        intrusive_ptr_add_ref(&m_MagneticField); addInputPort(&m_MagneticField);
        intrusive_ptr_add_ref(&m_RelativeAltitude); addInputPort(&m_RelativeAltitude);
        
        intrusive_ptr_add_ref(&m_Attitude); addOutputPort(&m_Attitude);
        intrusive_ptr_add_ref(&m_Position); addOutputPort(&m_Position);
        intrusive_ptr_add_ref(&m_Velocity); addOutputPort(&m_Velocity);
        intrusive_ptr_add_ref(&m_EarthMagneticField); addOutputPort(&m_EarthMagneticField);
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
            !m_MagneticField.sourceConnection() )
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
        control::PoseEstimation::unprepare(yctx);
    }
    
    #define EKF2_USE_VR 1
    #define EKF2_USE_RR 1
    #define EKF2_USE_MF 1
    #define EKF2_USE_BA 1
    
    system::error_code PoseEstimation::update(asio::yield_context yctx)
    {
        #if EKF2_USE_VR
        Vec3f velocityRate = Vec3f(
            -m_VelocityRate.m_Value(0),
            -m_VelocityRate.m_Value(1),
            -m_VelocityRate.m_Value(2)
        ) * 9.80665f;
        #else
        Vec3f velocityRate = Vec3f(
            0.0f,
            0.0f,
            -9.80665f
        );
        #endif
        
        #if EKF2_USE_RR
        Vec3f rotationRate = Vec3f(
             m_RotationRate.m_Value(0),
             m_RotationRate.m_Value(1),
             m_RotationRate.m_Value(2)
        );
        #else
        Vec3f rotationRate = Vec3f::Zero();
        #endif
        
        #if EKF2_USE_MF
        Vec3f magneticField = Vec3f(
             -m_MagneticField.m_Value(0),
             -m_MagneticField.m_Value(1),
             -m_MagneticField.m_Value(2)
        );
        #else
        Vec3f magneticField = Vec3f::Zero();
        #endif
        
        #if EKF2_USE_BA
        float baroHeight = m_RelativeAltitude.m_Value;
        #else
        float baroHeight = 0.0f;
        #endif
        
        
        if(!m_LastRatesInitialized)
        {
            m_LastRatesInitialized = true;
            m_LastVelocityRate = velocityRate;
            m_LastRotationRate = rotationRate;
        }
        
        uint64_t const absTus = std::lround(1.0e+6f*m_Time.m_Value);
        uint64_t const dTus = std::lround(1.0e+6f*m_DT.m_Value);
        
        
        Vec3f imuVelRateData = 0.5f*(velocityRate + m_LastVelocityRate)*m_DT.m_Value;
        Vec3f imuRotRateData = 0.5f*(rotationRate + m_LastRotationRate)*m_DT.m_Value;
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
        m_Attitude.m_Value = Quaternionf(estData[0], estData[1], estData[2], estData[3]);
        
        m_EKF.copy_position(estData);
        m_Position.m_Value = -Vec3f(estData[0], estData[1], estData[2]);
        
        m_EKF.copy_velocity(estData);
        m_Velocity.m_Value = -Vec3f(estData[0], estData[1], estData[2]);

        
        m_EKF.get_heading_innov(estData);
        m_EarthMagneticField.m_Value = Vec3f(estData[0], estData[0], estData[0]);
        
        m_LastVelocityRate = velocityRate;
        m_LastRotationRate = rotationRate;
        
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ekf2
} //namespace control
} //namespace uquad
