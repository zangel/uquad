#ifndef UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H
#define UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "EKF.h"


namespace uquad
{
namespace control
{
namespace ekf2
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
        
        EKF m_EKF;
        
        base::TimePoint m_IMUStartTime;
        base::TimePoint m_IMULastTime;
        
        Vec3f m_LastVelRate;
        Vec3f m_LastAngRate;
    };
    
    
} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H
