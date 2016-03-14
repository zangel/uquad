#include "PoseEstimation.h"
#include "../SystemLibrary.h"
#include "../../base/Error.h"
#include "fusion.h"

#define UQUAD_MAG_COUNTSPERUT           6
#define UQUAD_ACC_COUNTSPERG            8192
#define UQUAD_GYR_COUNTSPERDEGPERSEC    64
#define UQUAD_BAR_MPERCOUNT             4
#define UQUAD_BAR_CPERCOUNT             256

namespace uquad
{
namespace control
{
namespace ikf
{
    PoseEstimation::Registry::Registry()
        : control::PoseEstimation::Registry("ikf")
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
        
        m_LoopCount = 0;
        
        //pressure sensor
        //m_PressureSensor.fmPerCount = UQUAD_BAR_MPERCOUNT;
        //m_PressureSensor.fCPerCount = UQUAD_BAR_CPERCOUNT;
    
        //accel sensor init
        m_AccelSensor.iCountsPerg = UQUAD_ACC_COUNTSPERG;
        m_AccelSensor.fgPerCount = 1.0F / UQUAD_ACC_COUNTSPERG;
        
        //gyro sensor init
        m_GyroSensor.iCountsPerDegPerSec = UQUAD_GYR_COUNTSPERDEGPERSEC;
		m_GyroSensor.fDegPerSecPerCount = 1.0F / UQUAD_GYR_COUNTSPERDEGPERSEC;
        
        #if UQUAD_CONTROL_IKF_POSE_ESTIMATION_USE_9DOF
        //mag sensor init
        m_MagSensor.iCountsPeruT = UQUAD_MAG_COUNTSPERUT;
        m_MagSensor.fCountsPeruT = (float)UQUAD_MAG_COUNTSPERUT;
        m_MagSensor.fuTPerCount = 1.0F / UQUAD_MAG_COUNTSPERUT;
        
        for(int i = CHX; i <= CHZ; i++)
            m_MagSensor.iBcAvg[i]= 0;
        
        fInitMagCalibration(&m_MagCalibration, &m_MagBuffer);
        
        m_9DOFState.iSensorFS = 100;
        m_9DOFState.iOversampleRatio = 1;
        
        m_9DOFState.resetflag = true;
        fInit_9DOF_GBY_KALMAN(&m_9DOFState, &m_AccelSensor, &m_MagSensor, &m_MagCalibration);
        #else
        
        m_6DOFState.iSampleFS = 100;
        m_6DOFState.iOversampleRatio = 1;
        m_6DOFState.resetflag = true;
        fInit_6DOF_GY_KALMAN(&m_6DOFState, &m_AccelSensor);
        #endif
        
        
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
        m_AccelSensor.iGs[CHX] =  m_AccelSensor.iCountsPerg*usd.velocityRate(0);
        m_AccelSensor.iGs[CHY] =  m_AccelSensor.iCountsPerg*usd.velocityRate(1);
        m_AccelSensor.iGs[CHZ] =  m_AccelSensor.iCountsPerg*usd.velocityRate(2);
        
        m_GyroSensor.iYs[CHX] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(0)*F180OVERPI;
        m_GyroSensor.iYs[CHY] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(1)*F180OVERPI;
        m_GyroSensor.iYs[CHZ] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(2)*F180OVERPI;
        
        
        
        //accel
        for(int i = CHX; i <= CHZ; i++)
        {
            m_AccelSensor.iGsBuffer[0][i] = m_AccelSensor.iGsAvg[i] = m_AccelSensor.iGs[i];
            m_AccelSensor.fGsAvg[i] = (float)m_AccelSensor.iGsAvg[i] * m_AccelSensor.fgPerCount;
        }
        
        //gyro
        for(int i = CHX; i <= CHZ; i++)
        {
            m_GyroSensor.iYsBuffer[0][i] = m_GyroSensor.iYs[i];
        }
        
        #if UQUAD_CONTROL_IKF_POSE_ESTIMATION_USE_9DOF

        m_MagSensor.iBs[CHX] = m_MagSensor.iCountsPeruT*usd.magneticField(0);
        m_MagSensor.iBs[CHY] = m_MagSensor.iCountsPeruT*usd.magneticField(1);
        m_MagSensor.iBs[CHZ] = m_MagSensor.iCountsPeruT*usd.magneticField(2);

        //mag
        for(int i = CHX; i <= CHZ; i++)
        {
            m_MagSensor.iBsBuffer[0][i] = m_MagSensor.iBs[i];
        }
        
        iUpdateMagnetometerBuffer(&m_MagBuffer, &m_MagSensor, m_LoopCount);
        
        
        for(int i = CHX; i <= CHZ; i++)
        {
            m_MagSensor.iBsAvg[i] = m_MagSensor.iBsBuffer[0][i];
            m_MagSensor.fBsAvg[i] = (float)m_MagSensor.iBsAvg[i] * m_MagSensor.fuTPerCount;
        }
        
        fInvertMagCal(&m_MagSensor, &m_MagCalibration);
        
        fRun_9DOF_GBY_KALMAN(&m_9DOFState, &m_AccelSensor, &m_MagSensor, &m_GyroSensor, &m_MagCalibration);
        
        
        if( (!m_MagCalibration.iMagCalHasRun && (m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS7CAL) && !(m_LoopCount % INTERVAL4CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS7CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL7CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL10CAL)) )
        {
            m_MagCalibration.iMagCalHasRun = true;
            fRunMagCalibration(&m_9DOFState, &m_MagCalibration, &m_MagBuffer, &m_MagSensor);
        }
        
        attitude = Quaternionf(m_9DOFState.fqPl.q0, m_9DOFState.fqPl.q1, m_9DOFState.fqPl.q2, m_9DOFState.fqPl.q3);
        velocity = Vec3f(m_9DOFState.fVelGl[0], m_9DOFState.fVelGl[1], m_9DOFState.fVelGl[2]);
        position.setZero();
        position.x() = m_MagCalibration.fFitErrorpc;
        bodyMagneticField = Vec3f(m_MagSensor.fBcAvg[0], m_MagSensor.fBcAvg[1], m_MagSensor.fBcAvg[2]);
        
        position = Vec3f(m_9DOFState.fDisGl[0], m_9DOFState.fDisGl[1], m_9DOFState.fDisGl[2]);
        #else
        
        fRun_6DOF_GY_KALMAN(&m_6DOFState, &m_AccelSensor, &m_GyroSensor);
        attitude = Quaternionf(m_6DOFState.fqPl.q0, m_6DOFState.fqPl.q1, m_6DOFState.fqPl.q2, m_6DOFState.fqPl.q3);
        #endif
        
        m_LoopCount++;
        
        
        //velocity = Vec3f(m_9DOFState.fRVecPl[0], m_9DOFState.fRVecPl[1], m_9DOFState.fRVecPl[2]);
        //position = Vec3f(m_9DOFState.fDisGl[0], m_9DOFState.fDisGl[1], m_9DOFState.fDisGl[2]);
        //bodyMagneticField = Vec3f(m_9DOFState.fAccGl[0], m_9DOFState.fAccGl[1], m_9DOFState.fAccGl[2]);
        //bodyMagneticField = Vec3f(m_MagCalibration.fV[0], m_MagCalibration.fV[1], m_MagCalibration.fV[2]);
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ikf
} //namespace control
} //namespace uquad
