#ifndef UQUAD_CONTROL_BLOCKS_PE_EKF_POSE_ESTIMATION_H
#define UQUAD_CONTROL_BLOCKS_PE_EKF_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "estimator_22states.h"


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
        DefaultInputSignal<DT> m_DT;
        DefaultInputSignal<Time> m_Time;
        DefaultInputSignal<VelocityRate> m_VelocityRate;
        DefaultInputSignal<RotationRate> m_RotationRate;
        DefaultInputSignal<MagneticField> m_MagneticField;
        DefaultInputSignal<Altitude> m_RelativeAltitude;
        
        DefaultOutputSignal<Attitude> m_Attitude;
        DefaultOutputSignal<Position> m_Position;
        DefaultOutputSignal<Velocity> m_Velocity;
        
        AttPosEKF::Context m_EKFContext;
        AttPosEKF m_EKF;
        
        base::TimeDuration m_MagDelay;
        
        bool m_bOnGround;

        float m_EKFCovariancePredDT;
    };
    
    
} //namespace ekf
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_BLOCKS_PE_EKF_POSE_ESTIMATION_H
