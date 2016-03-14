#include "PoseEstimation.h"
#include "SystemLibrary.h"

namespace uquad
{
namespace control
{
    PoseEstimation::Registry::Registry(std::string const &n)
        : base::Object::Registry(n, typeid(PoseEstimation))
    {
    }
    
    PoseEstimation::PoseEstimation()
        : System()
    {
    }
    
    PoseEstimation::~PoseEstimation()
    {
    }

} //namespace control
} //namespace uquad

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_EKF)
#include "ekf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf::PoseEstimation, uquad::control::SystemLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_EKF2)
#include "ekf2/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf2::PoseEstimation, uquad::control::SystemLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_IKF)
#include "ikf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ikf::PoseEstimation, uquad::control::SystemLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_MADGWICK)
#include "madgwick/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::madgwick::PoseEstimation, uquad::control::SystemLibrary)
#endif
