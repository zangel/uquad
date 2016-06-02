#include "PoseEstimation.h"
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
namespace blocks
{
namespace pe
{
namespace ikf
{

    PoseEstimation::Registry::Registry()
        : control::blocks::PoseEstimation::Registry("ikf")
    {
    }
    
    intrusive_ptr<base::Object> PoseEstimation::Registry::createObject() const
    {
        return intrusive_ptr<base::Object>(new PoseEstimation());
    }
    
    PoseEstimation::PoseEstimation()
        : control::blocks::PoseEstimation()
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
            !m_ISVelocityRate.isConnected() ||
            !m_ISRotationRate.isConnected() ||
            !m_ISMagneticField.isConnected() )
        {
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_LoopCount = 0;
        
        m_MagnetometerCalibration.reset();
        
        
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
        m_9DOFState.resetflag = true;
        #else
        m_6DOFState.resetflag = true;
        #endif
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void PoseEstimation::unprepare(asio::yield_context yctx)
    {
        control::blocks::PoseEstimation::unprepare(yctx);
    }
    
    system::error_code PoseEstimation::update(asio::yield_context yctx)
    {
        m_AccelSensor.iGs[CHX] =  m_AccelSensor.iCountsPerg*m_ISVelocityRate.m_Value(0);
        m_AccelSensor.iGs[CHY] =  m_AccelSensor.iCountsPerg*m_ISVelocityRate.m_Value(1);
        m_AccelSensor.iGs[CHZ] =  m_AccelSensor.iCountsPerg*m_ISVelocityRate.m_Value(2);
        
        m_GyroSensor.iYs[CHX] =  m_GyroSensor.iCountsPerDegPerSec*m_ISRotationRate.m_Value(0)*F180OVERPI;
        m_GyroSensor.iYs[CHY] =  m_GyroSensor.iCountsPerDegPerSec*m_ISRotationRate.m_Value(1)*F180OVERPI;
        m_GyroSensor.iYs[CHZ] =  m_GyroSensor.iCountsPerDegPerSec*m_ISRotationRate.m_Value(2)*F180OVERPI;
        
        //accel
        for(int i = CHX; i <= CHZ; i++)
        {
            m_AccelSensor.iGsBuffer[i] = m_AccelSensor.iGsAvg[i] = m_AccelSensor.iGs[i];
            m_AccelSensor.fGsAvg[i] = (float)m_AccelSensor.iGsAvg[i] * m_AccelSensor.fgPerCount;
        }
        
        //gyro
        for(int i = CHX; i <= CHZ; i++)
        {
            m_GyroSensor.iYsBuffer[i] = m_GyroSensor.iYs[i];
        }
        
#if UQUAD_CONTROL_IKF_POSE_ESTIMATION_USE_9DOF
        m_9DOFState.fSensorDT = m_ISDT.m_Value;
        
        m_MagSensor.iBs[CHX] = m_MagSensor.iCountsPeruT*m_ISMagneticField.m_Value(0);
        m_MagSensor.iBs[CHY] = m_MagSensor.iCountsPeruT*m_ISMagneticField.m_Value(1);
        m_MagSensor.iBs[CHZ] = m_MagSensor.iCountsPeruT*m_ISMagneticField.m_Value(2);

        //mag
        for(int i = CHX; i <= CHZ; i++)
        {
            m_MagSensor.iBsBuffer[i] = m_MagSensor.iBs[i];
        }
        
        iUpdateMagnetometerBuffer(&m_MagBuffer, &m_MagSensor, m_LoopCount);
        m_MagnetometerCalibration.update(m_ISMagneticField.m_Value, m_ISDT.m_Value);
        
        for(int i = CHX; i <= CHZ; i++)
        {
            m_MagSensor.iBsAvg[i] = m_MagSensor.iBsBuffer[i];
            m_MagSensor.fBsAvg[i] = (float)m_MagSensor.iBsAvg[i] * m_MagSensor.fuTPerCount;
        }
        
        fInvertMagCal(&m_MagSensor, &m_MagCalibration);
        
        fRun_9DOF_GBY_KALMAN(&m_9DOFState, &m_AccelSensor, &m_MagSensor, &m_GyroSensor, &m_MagCalibration);
        
        
       if( /*(!m_MagCalibration.iMagCalHasRun && */(m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL)/*) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS4CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS7CAL) && !(m_LoopCount % INTERVAL4CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS7CAL) && (m_MagBuffer.iMagBufferCount < MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL7CAL)) ||
            ((m_MagBuffer.iMagBufferCount >= MINMEASUREMENTS10CAL) && !(m_LoopCount % INTERVAL10CAL)) */)
        {
            m_MagCalibration.iMagCalHasRun = true;
            fRunMagCalibration(&m_9DOFState, &m_MagCalibration, &m_MagBuffer, &m_MagSensor);
        }
        
        
        m_OSAttitude.m_Value = Quaternionf(m_9DOFState.fqPl.q0, m_9DOFState.fqPl.q1, m_9DOFState.fqPl.q2, m_9DOFState.fqPl.q3);
        m_OSPosition.m_Value = Vec3f(m_9DOFState.fAccGl[0], m_9DOFState.fAccGl[1], m_9DOFState.fAccGl[2]);
        m_OSVelocity.m_Value = Vec3f(m_9DOFState.fVelGl[0], m_9DOFState.fVelGl[1], m_9DOFState.fVelGl[2]);
        
        //m_RotationRateEstimated.m_Value = FPIOVER180*Vec3f(m_9DOFState.fOmega[0], m_9DOFState.fOmega[1], m_9DOFState.fOmega[2]);
        
        #else
        m_6DOFState.fSensorDT = m_DT.m_Value;
        
        fRun_6DOF_GY_KALMAN(&m_6DOFState, &m_AccelSensor, &m_GyroSensor);
        m_OSAttitude.m_Value = Quaternionf(m_6DOFState.fqPl.q0, m_6DOFState.fqPl.q1, m_6DOFState.fqPl.q2, m_6DOFState.fqPl.q3);
        #endif
        
        m_LoopCount++;
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace ikf
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad
