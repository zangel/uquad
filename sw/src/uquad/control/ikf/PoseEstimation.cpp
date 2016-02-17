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
        
        //mag sensor init
        m_MagSensor.iCountsPeruT = UQUAD_MAG_COUNTSPERUT;
        m_MagSensor.fCountsPeruT = (float)UQUAD_MAG_COUNTSPERUT;
        m_MagSensor.fuTPerCount = 1.0F / UQUAD_MAG_COUNTSPERUT;
        
        for(int i = CHX; i <= CHZ; i++)
            m_MagSensor.iBcAvg[i]= 0;
        
        fInitMagCalibration(&m_MagCalibration, &m_MagBuffer);
        
        m_9DOFState.resetflag = true;
        fInit_9DOF_GBY_KALMAN(&m_9DOFState, &m_AccelSensor, &m_MagSensor, &m_MagCalibration);
        
        m_6DOFState.resetflag = true;
        fInit_6DOF_GY_KALMAN(&m_6DOFState, &m_AccelSensor);
        
        
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
#if THISCOORDSYSTEM == NED
        m_AccelSensor.iGs[CHX] = m_AccelSensor.iCountsPerg*usd.velocityRate(0)/GTOMSEC2;
        m_AccelSensor.iGs[CHY] = m_AccelSensor.iCountsPerg*usd.velocityRate(1)/GTOMSEC2;
        m_AccelSensor.iGs[CHZ] = m_AccelSensor.iCountsPerg*usd.velocityRate(2)/GTOMSEC2;
#elif THISCOORDSYSTEM == ANDROID
        m_AccelSensor.iGs[CHX] = -m_AccelSensor.iCountsPerg*usd.velocityRate(0)/GTOMSEC2;
        m_AccelSensor.iGs[CHY] = -m_AccelSensor.iCountsPerg*usd.velocityRate(1)/GTOMSEC2;
        m_AccelSensor.iGs[CHZ] = -m_AccelSensor.iCountsPerg*usd.velocityRate(2)/GTOMSEC2;
#elif THISCOORDSYSTEM == WIN8
        m_AccelSensor.iGs[CHX] =  m_AccelSensor.iCountsPerg*usd.velocityRate(0)/GTOMSEC2;
        m_AccelSensor.iGs[CHY] =  m_AccelSensor.iCountsPerg*usd.velocityRate(1)/GTOMSEC2;
        m_AccelSensor.iGs[CHZ] = -m_AccelSensor.iCountsPerg*usd.velocityRate(2)/GTOMSEC2;
#endif


#if THISCOORDSYSTEM == NED
        m_GyroSensor.iYs[CHX] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(0)*F180OVERPI;
        m_GyroSensor.iYs[CHY] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(1)*F180OVERPI;
        m_GyroSensor.iYs[CHZ] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(2)*F180OVERPI;
#elif THISCOORDSYSTEM == ANDROID
        m_GyroSensor.iYs[CHX] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(0)*F180OVERPI;
        m_GyroSensor.iYs[CHY] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(1)*F180OVERPI;
        m_GyroSensor.iYs[CHZ] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(2)*F180OVERPI;
#elif THISCOORDSYSTEM == WIN8
        m_GyroSensor.iYs[CHX] = -m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(0)*F180OVERPI;
        m_GyroSensor.iYs[CHY] = -m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(1)*F180OVERPI;
        m_GyroSensor.iYs[CHZ] =  m_GyroSensor.iCountsPerDegPerSec*usd.rotationRate(2)*F180OVERPI;
#endif


#if THISCOORDSYSTEM == NED
        m_MagSensor.iBs[CHX] = m_MagSensor.iCountsPeruT*usd.magneticField(0);
        m_MagSensor.iBs[CHY] = m_MagSensor.iCountsPeruT*usd.magneticField(1);
        m_MagSensor.iBs[CHZ] = m_MagSensor.iCountsPeruT*usd.magneticField(2);
#elif THISCOORDSYSTEM == ANDROID
        m_MagSensor.iBs[CHX] =  m_MagSensor.iCountsPeruT*usd.magneticField(0);
        m_MagSensor.iBs[CHY] =  m_MagSensor.iCountsPeruT*usd.magneticField(1);
        m_MagSensor.iBs[CHZ] =  m_MagSensor.iCountsPeruT*usd.magneticField(2);
#elif THISCOORDSYSTEM == WIN8
        m_MagSensor.iBs[CHX] = -m_MagSensor.iCountsPeruT*usd.magneticField(0);
        m_MagSensor.iBs[CHY] = -m_MagSensor.iCountsPeruT*usd.magneticField(1);
        m_MagSensor.iBs[CHZ] =  m_MagSensor.iCountsPeruT*usd.magneticField(2);
#endif

        //accel
        for(int i = CHX; i <= CHZ; i++)
        {
            m_AccelSensor.iGsBuffer[0][i] = m_AccelSensor.iGsAvg[i] = m_AccelSensor.iGs[i];
            m_AccelSensor.fGsAvg[i] = (float)m_AccelSensor.iGsAvg[i] * m_AccelSensor.fgPerCount;
        }
        
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
        
        //gyro
        for(int i = CHX; i <= CHZ; i++)
        {
            m_GyroSensor.iYsBuffer[0][i] = m_GyroSensor.iYs[i];
        }
        
        fRun_9DOF_GBY_KALMAN(&m_9DOFState, &m_AccelSensor, &m_MagSensor, &m_GyroSensor, &m_MagCalibration);
        
        
        if( (!m_MagCalibration.iMagCalHasRun && (m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL)) /*||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS7CAL) && !(m_LoopCount % INTERVAL4CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS7CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL7CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL10CAL))*/ )
        {
            m_MagCalibration.iMagCalHasRun = true;
            fRunMagCalibration(&m_MagCalibration, &m_MagBuffer, &m_MagSensor);
        }
        
        
        fRun_6DOF_GY_KALMAN(&m_6DOFState, &m_AccelSensor, &m_GyroSensor);
        
        m_LoopCount++;
        
        #if 1
        attitude = Quaternionf(m_9DOFState.fqPl.q0, m_9DOFState.fqPl.q1, m_9DOFState.fqPl.q2, m_9DOFState.fqPl.q3);
        #else
        attitude = Quaternionf(m_6DOFState.fqPl.q0, m_6DOFState.fqPl.q1, m_6DOFState.fqPl.q2, m_6DOFState.fqPl.q3);
        #endif
        
        velocity = Vec3f(m_9DOFState.fVelGl[0], m_9DOFState.fVelGl[1], m_9DOFState.fVelGl[2]);
        //velocity = Vec3f(m_9DOFState.fRVecPl[0], m_9DOFState.fRVecPl[1], m_9DOFState.fRVecPl[2]);
        
        //position = Vec3f(m_9DOFState.fDisGl[0], m_9DOFState.fDisGl[1], m_9DOFState.fDisGl[2]);
        position.setZero();
        position.x() = m_9DOFState.fRhoPl;
        position.y() = m_9DOFState.fChiPl;
        //position.x() = m_MagCalibration.fFitErrorpc;
        
        bodyMagneticField = Vec3f(m_9DOFState.fAccGl[0], m_9DOFState.fAccGl[1], m_9DOFState.fAccGl[2]);
        
        

        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ikf
} //namespace control
} //namespace uquad

UQUAD_BASE_REGISTER_OBJECT(uquad::control::ikf::PoseEstimation, uquad::control::SystemLibrary)
