#ifndef UQUAD_CONTROL_BLOCKS_PE_EKF2_POSE_ESTIMATION_H
#define UQUAD_CONTROL_BLOCKS_PE_EKF2_POSE_ESTIMATION_H

#include "../../PoseEstimation.h"
//#include "EKF.h"
#include "ecl/EKF/ekf.h"

namespace uquad
{
namespace control
{
namespace blocks
{
namespace pe
{
namespace ekf2
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
        bool m_bOnGround;
        bool m_bArmed;
        
        ::Ekf m_EKF;
        
        bool m_LastRatesInitialized;
        Vec3f m_LastVelocityRate;
        Vec3f m_LastRotationRate;
    };
    

} //namespace ekf2
} //namespace pe
} //namespace blocks
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_BLOCKS_PE_EKF2_POSE_ESTIMATION_H
