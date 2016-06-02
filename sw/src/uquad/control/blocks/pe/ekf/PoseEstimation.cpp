#include "PoseEstimation.h"

namespace uquad
{
namespace control
{
namespace blocks
{
namespace pe
{
namespace ekf
{
    
    PoseEstimation::Registry::Registry()
        : control::blocks::PoseEstimation::Registry("ekf")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::blocks::PoseEstimation()
        , m_EKFContext( { 0 } )
        , m_EKF(m_EKFContext)
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
        
        if( !m_DT.sourceConnection() ||
            !m_VelocityRate.sourceConnection() ||
            !m_RotationRate.sourceConnection() ||
            !m_MagneticField.sourceConnection() )
        {
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_EKFContext.time_us = 0;
        
        m_EKF.useCompass = true;
        
        m_MagDelay = chrono::milliseconds(30);
        
        m_bOnGround = true;
        
        m_EKFCovariancePredDT = 0.0f;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void PoseEstimation::unprepare(asio::yield_context yctx)
    {
        control::blocks::PoseEstimation::unprepare(yctx);
    }
    
    #define EKF_USE_VR 1
    #define EKF_USE_RR 1
    #define EKF_USE_MF 0
    #define EKF_USE_BA 0
    
    system::error_code PoseEstimation::update(asio::yield_context yctx)
    {
        #if EKF_USE_VR
        Vector3f velocityRate = Vector3f(
            -m_VelocityRate.m_Value(0),
            -m_VelocityRate.m_Value(1),
            -m_VelocityRate.m_Value(2)
        ) * 9.80665f;
        #else
        Vector3f velocityRate = Vector3f(0.0f, 0.0f, -9.80665f);
        #endif
        
        #if EKF_USE_RR
        Vector3f rotationRate = Vector3f(
            m_RotationRate.m_Value(0),
            m_RotationRate.m_Value(1),
            m_RotationRate.m_Value(2)
        );
        #else
        Vector3f rotationRate = Vector3f();
        #endif
        
        #if EKF_USE_MF
        Vector3f magneticField = Vector3f(
            -m_MagneticField.m_Value(0),
            -m_MagneticField.m_Value(1),
            -m_MagneticField.m_Value(2)
        );
        #else
        Vector3f magneticField = Vector3f();//ekf22::Vector3f(0.0f, 10.0f, 0.0f);
        #endif
        
        
        #if EKF_USE_BA
        float baroHeight = -m_RelativeAltitude.m_Value;
        #else
        float baroHeight = 0.0f;
        #endif
        
        m_EKFContext.time_us = std::lround(1.0e+6f*m_Time.m_Value);
        uint32_t const time_ms = m_EKFContext.getMillis();
        
        
        m_EKF.dtIMU = m_DT.m_Value;
        
        m_EKF.dAngIMU = 0.5f*(m_EKF.angRate + rotationRate)*m_EKF.dtIMU;
        m_EKF.angRate = rotationRate;
        
        m_EKF.dVelIMU = 0.5f*(m_EKF.accel + velocityRate)*m_EKF.dtIMU;
        m_EKF.accel = velocityRate;
        
        m_EKF.magData = magneticField;
        
        m_EKF.updateDtHgtFilt(m_DT.m_Value);
        m_EKF.baroHgt = baroHeight;
        
        if(!m_EKF.statesInitialised)
        {
            float initVelNED[3] = {0.0f, 0.0f, 0.0f};
            m_EKF.posNE[0] = 0.0f;
            m_EKF.posNE[1] = 0.0f;
        
            m_EKF.InitialiseFilter(initVelNED, 0.0, 0.0, 0.0f, 0.0f);
        }
        
        m_EKF.setOnGround(m_bOnGround);
        
        
        struct ekf_status_report ekfReport;
        int ekfCheck = m_EKF.CheckAndBound(&ekfReport);
        
        if(ekfCheck)
        {
            return base::makeErrorCode(base::kENoError);
        }
        
        //IMU update
        m_EKF.UpdateStrapdownEquationsNED();
        m_EKF.StoreStates(time_ms);
        
        m_EKF.summedDelAng = m_EKF.summedDelAng + m_EKF.correctedDelAng;
        m_EKF.summedDelVel = m_EKF.summedDelVel + m_EKF.dVelIMU;
        m_EKFCovariancePredDT += m_EKF.dtIMU;
        
        if( (m_EKFCovariancePredDT >= (m_EKF.covTimeStepMax - m_EKF.dtIMU)) ||
            (m_EKF.summedDelAng.length() > m_EKF.covDelAngMax) )
        {
            m_EKF.CovariancePrediction(m_EKFCovariancePredDT);
            m_EKF.summedDelAng.zero();
            m_EKF.summedDelVel.zero();
            m_EKFCovariancePredDT = 0.0f;
        }
        
        m_EKF.globalTimeStamp_ms = time_ms;
        
        //opt flow fusion
        m_EKF.fuseOptFlowData = false;
        
        //gps fusion
        m_EKF.fuseVelData = false;
		m_EKF.fusePosData = false;
        
        //barometer fusion
        m_EKF.hgtMea = m_EKF.baroHgt;
        m_EKF.fuseHgtData = true;
        
        // recall states stored at time of measurement after adjusting for delays
		m_EKF.RecallStates(m_EKF.statesAtHgtTime, time_ms - 20);

		// run the fusion step
		m_EKF.FuseVelposNED();
        
        //m_EKF.globalTimeStamp_ms = chrono::duration_cast<chrono::milliseconds>(m_IMUDuration).count();
    
        
        #if 0
        //mag fusion
        m_EKF.fuseMagData = true;
		m_EKF.RecallStates(m_EKF.statesAtMagMeasTime, time_ms - 20); // Assume 50 msec avg delay for magnetometer data

		m_EKF.magstate.obsIndex = 0;
		m_EKF.FuseMagnetometer();
		m_EKF.FuseMagnetometer();
		m_EKF.FuseMagnetometer();
        #else
        m_EKF.fuseMagData = false;
        #endif
        
        //airspeed fusion
        m_EKF.fuseVtasData = false;
        
        m_Attitude.m_Value = Quaternionf(m_EKF.states[0], m_EKF.states[1], m_EKF.states[2], m_EKF.states[3]);
        m_Velocity.m_Value = Vec3f(m_EKF.states[4], m_EKF.states[5], m_EKF.states[6]);
        m_Position.m_Value = Vec3f(m_EKF.states[7], m_EKF.states[8], m_EKF.states[9]);
        
        //roationRateBias = Vec3f(m_EKF.states[10], m_EKF.states[11], m_EKF.states[12]);
        //velocityRateZBias = m_EKF.states[13];
        //wind = Vec2f(m_EKF.states[14], m_EKF.states[15]);
        
        //earthMagneticField = Vec3f(m_EKF.states[16], m_EKF.states[17], m_EKF.states[18]);
        //bodyMagneticField = Vec3f(m_EKF.states[19], m_EKF.states[20], m_EKF.states[21]);
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ekf
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad
