#ifndef UQUAD_CONTROL_MADGWICK_POSE_ESTIMATION_H
#define UQUAD_CONTROL_MADGWICK_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "imu_filter.h"

namespace uquad
{
namespace control
{
namespace madgwick
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
        Quaternionf computeOrientation(Vec3f const &vr, Vec3f const &mf);
        
        
    private:
        bool m_bPrepared;
        
        ImuFilter m_IMUFilter;
        bool m_IMUFilterInitialized;
        
        base::TimePoint m_IMUStartTime;
        base::TimePoint m_IMULastTime;
        
        Vec3f m_MagneticBias;
    };

} //namespace madgwick
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_MADGWICK_POSE_ESTIMATION_H
