#ifndef UQUAD_CONTROL_EKF_POSE_ESTIMATION_H
#define UQUAD_CONTROL_EKF_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "estimator_22states.h"


namespace uquad
{
namespace control
{
namespace ekf
{
    class PoseEstimation
        : public control::PoseEstimation
    {
    public:
        
        class Registry
			: public control::PoseEstimation::Registry
		{
		public:
			Registry();

			intrusive_ptr<base::Object> createObject() const;
		};
        
        PoseEstimation();
        ~PoseEstimation();
        
        bool isValid() const;
        
        system::error_code prepare();
        bool isPrepared() const;
        void unprepare();
        
        system::error_code processUQuadSensorsData(hal::UQuadSensorsData const &usd);
        
    private:
        bool m_bPrepared;
        
        ::AttPosEKF::Context m_EKFContext;
        ::AttPosEKF m_EKF;
        
        base::TimePoint m_IMUStartTime;
        base::TimePoint m_IMUTime;
                
        base::TimeDuration m_MagDelay;
        
        bool m_bOnGround;

        float m_EKFCovariancePredDT;
    };
    
    
} //namespace control
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF_POSE_ESTIMATION_H
