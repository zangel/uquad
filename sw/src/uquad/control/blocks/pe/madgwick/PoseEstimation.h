#ifndef UQUAD_CONTROL_BLOCKS_PE_MADGWICK_POSE_ESTIMATION_H
#define UQUAD_CONTROL_BLOCKS_PE_MADGWICK_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
#include "imu_filter.h"

namespace uquad
{
namespace control
{
namespace blocks
{
namespace pe
{
namespace madgwick
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
        //Quaternionf computeOrientation(Vec3f const &vr, Vec3f const &mf);
        
        
    private:
        DefaultInputSignal<DT> m_DT;
        DefaultInputSignal<VelocityRate> m_VelocityRate;
        DefaultInputSignal<RotationRate> m_RotationRate;
        DefaultInputSignal<MagneticField> m_MagneticField;
        
        DefaultOutputSignal<Attitude> m_Attitude;
        
        ImuFilter m_IMUFilter;
        bool m_IMUFilterInitialized;
    };

} //namespace madgwick
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_BLOCKS_PE_MADGWICK_POSE_ESTIMATION_H
