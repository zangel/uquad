#ifndef UQUAD_CONTROL_BLOCKS_PE_IKF_POSE_ESTIMATION_H
#define UQUAD_CONTROL_BLOCKS_PE_IKF_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "../../../MagnetometerCalibration.h"

#include "magnetic.h"
#include "types.h"

#define UQUAD_CONTROL_IKF_POSE_ESTIMATION_USE_9DOF 1

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

    class PoseEstimation
        : public control::blocks::PoseEstimation
    {
    public:
        
        class Registry
			: public control::blocks::PoseEstimation::Registry
		{
		public:
			Registry();

			intrusive_ptr<base::Object> createObject() const;
		};
        
        PoseEstimation();
        ~PoseEstimation();
        
        bool isValid() const;
        
        
    protected:
        system::error_code prepare(asio::yield_context yctx);
        void unprepare(asio::yield_context yctx);
        system::error_code update(asio::yield_context yctx);
        
    private:
        uint32_t m_LoopCount;
        
        AccelSensor m_AccelSensor;
        GyroSensor m_GyroSensor;
        
        #if UQUAD_CONTROL_IKF_POSE_ESTIMATION_USE_9DOF
        MagSensor m_MagSensor;
        MagneticBuffer m_MagBuffer;
        MagCalibration m_MagCalibration;
        SV_9DOF_GBY_KALMAN m_9DOFState;
        #else
        SV_6DOF_GY_KALMAN m_6DOFState;
        #endif
        //SV_1DOF_P_BASIC m_PressureState;
        
        MagnetometerCalibration m_MagnetometerCalibration;
    };
    
    
} //namespace ikf
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_BLOCKS_PE_IKF_POSE_ESTIMATION_H
