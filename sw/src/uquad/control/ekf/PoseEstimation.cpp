#include "PoseEstimation.h"
#include "../SystemLibrary.h"
#include "../../base/Error.h"

namespace uquad
{
namespace control
{
namespace ekf
{
    PoseEstimation::Registry::Registry()
        : control::PoseEstimation::Registry("ekf")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::PoseEstimation()
        , m_bPrepared(false)
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
        
    system::error_code PoseEstimation::prepare()
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_EKF.useCompass = true;
        
        m_IMUTime = m_IMUStartTime = base::TimePoint::min();
        m_MagDelay = chrono::milliseconds(30);
        
        m_bOnGround = true;
        
        m_EKFCovariancePredDT = 0.0f;
        
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
    
    #define EKF_USE_VR 1
    #define EKF_USE_AR 1
    #define EKF_USE_MF 1
    #define EKF_USE_BA 0
    
    system::error_code PoseEstimation::processUQuadSensorsData(hal::UQuadSensorsData const &usd)
    {
        #if EKF_USE_AR
        ::Vector3f angRate = ::Vector3f(
            usd.rotationRate(0),
            usd.rotationRate(1),
            usd.rotationRate(2)
        );
        #else
        ::Vector3f angRate = ::Vector3f();
        #endif
        
        #if EKF_USE_VR
        ::Vector3f velocityRate = ::Vector3f(
            usd.velocityRate(0),
            usd.velocityRate(1),
            usd.velocityRate(2)
        );
        #else
        ::Vector3f velocityRate = ::Vector3f(0.0f, 0.0f, -9.80665f);
        #endif
        
        #if EKF_USE_MF
        ::Vector3f magneticField = ::Vector3f(
            usd.magneticField(0),
            usd.magneticField(1),
            usd.magneticField(2)
        );
        #else
        ::Vector3f magneticField = ::Vector3f(0.0f, 10.0f, 0.0f);
        #endif
        
        
        #if EKF_USE_BA
        float baroHeight = usd.baroPressure;
        #else
        float baroHeight = 10.0f;
        #endif
        
        
        if(m_IMUStartTime == base::TimePoint::min())
        {
            m_IMUStartTime = m_IMUTime = usd.time;
            m_EKF.angRate = angRate;
            m_EKF.accel = velocityRate;
            m_EKF.magData = magneticField;
            
            return base::makeErrorCode(base::kENoError);
        }
        
        base::TimeDuration dT = usd.time - m_IMUTime;
        base::TimeDuration time = usd.time - m_IMUStartTime;
        m_IMUTime = usd.time;
        
        m_EKFContext.time_us = chrono::duration_cast<chrono::microseconds>(time).count() + 50000;
        uint32_t const time_ms = m_EKFContext.getMillis();
        float deltaT = 1.0e-6f*chrono::duration_cast<chrono::microseconds>(dT).count();
        
        m_EKF.dtIMU = deltaT;
        
        m_EKF.dAngIMU = 0.5f*(m_EKF.angRate + angRate)*m_EKF.dtIMU;
        m_EKF.angRate = angRate;
        
        m_EKF.dVelIMU = 0.5f*(m_EKF.accel + velocityRate)*m_EKF.dtIMU;
        m_EKF.accel = velocityRate;
        
        m_EKF.magData = magneticField;
        
        m_EKF.updateDtHgtFilt(deltaT);
        m_EKF.baroHgt = baroHeight;
        
        m_EKF.setOnGround(m_bOnGround);
        
        if(!m_EKF.statesInitialised)
        {
            float initVelNED[3] = {0.0f, 0.0f, 0.0f};
            m_EKF.posNE[0] = 0.0f;
            m_EKF.posNE[1] = 0.0f;
        
            m_EKF.InitialiseFilter(initVelNED, 0.0, 0.0, 0.0f, 0.0f);
        }
        
        struct ::ekf_status_report ekfReport;
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
    
        
        //mag fusion
        m_EKF.fuseMagData = true;
		m_EKF.RecallStates(m_EKF.statesAtMagMeasTime, time_ms - 20); // Assume 50 msec avg delay for magnetometer data

		m_EKF.magstate.obsIndex = 0;
		m_EKF.FuseMagnetometer();
		m_EKF.FuseMagnetometer();
		m_EKF.FuseMagnetometer();
        
        //airspeed fusion
        m_EKF.fuseVtasData = false;
        
        attitude = Quaternionf(m_EKF.states[0], m_EKF.states[1], m_EKF.states[2], m_EKF.states[3]);
        velocity = Vec3f(m_EKF.states[4], m_EKF.states[5], m_EKF.states[6]);
        position = Vec3f(m_EKF.states[7], m_EKF.states[8], m_EKF.states[9]);
        
        roationRateBias = Vec3f(m_EKF.states[10], m_EKF.states[11], m_EKF.states[12]);
        velocityRateZBias = m_EKF.states[13];
        wind = Vec2f(m_EKF.states[14], m_EKF.states[15]);
        
        earthMagneticField = Vec3f(m_EKF.states[16], m_EKF.states[17], m_EKF.states[18]);
        bodyMagneticField = Vec3f(m_EKF.states[19], m_EKF.states[20], m_EKF.states[21]);
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace control
} //namespace control
} //namespace uquad

UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf::PoseEstimation, uquad::control::SystemLibrary)