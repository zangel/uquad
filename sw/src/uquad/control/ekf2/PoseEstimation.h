#ifndef UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H
#define UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H

#include "../PoseEstimation.h"
//#include "EKF.h"
#include "ecl/EKF/ekf.h"

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
        DefaultOutputSignal<MagneticField> m_EarthMagneticField;
        
        bool m_bOnGround;
        bool m_bArmed;
        
        ::Ekf m_EKF;
        
        bool m_LastRatesInitialized;
        Vec3f m_LastVelocityRate;
        Vec3f m_LastRotationRate;
    };
    
    
} //namespace ekf2
} //namespace control
} //namespace uquad


#endif //UQUAD_CONTROL_EKF2_POSE_ESTIMATION_H
