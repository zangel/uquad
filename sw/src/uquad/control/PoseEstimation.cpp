#include "PoseEstimation.h"
#include "BlockLibrary.h"

namespace uquad
{
namespace control
{
    
    PoseEstimation::Registry::Registry(std::string const &n)
        : base::Object::Registry(n, typeid(PoseEstimation))
    {
    }
    
    std::string const& PoseEstimation::dTName()
    {
        static std::string _name = "dT";
        return _name;
    }
    
    std::string const& PoseEstimation::timeName()
    {
        static std::string _name = "time";
        return _name;
    }
    
    std::string const& PoseEstimation::velocityRateName()
    {
        static std::string _name = "velocityRate";
        return _name;
    }
    
    std::string const& PoseEstimation::rotationRateName()
    {
        static std::string _name = "rotationRate";
        return _name;
    }
    
    std::string const& PoseEstimation::magneticFieldName()
    {
        static std::string _name = "magneticField";
        return _name;
    }
    
    std::string const& PoseEstimation::relativeAltitudeName()
    {
        static std::string _name = "relativeAltitude";
        return _name;
    }
    
    std::string const& PoseEstimation::attitudeName()
    {
        static std::string _name = "attitude";
        return _name;
    }
    
    std::string const& PoseEstimation::positionName()
    {
        static std::string _name = "position";
        return _name;
    }
    
    std::string const& PoseEstimation::velocityName()
    {
        static std::string _name = "velocity";
        return _name;
    }
    
    std::string const& PoseEstimation::earthMagneticFieldName()
    {
        static std::string _name = "earthMagneticField";
        return _name;
    }
    
    PoseEstimation::PoseEstimation()
        : Block()
    {
    }
    
    PoseEstimation::~PoseEstimation()
    {
    }
    
} //namespace control
} //namespace uquad

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_EKF)
#include "ekf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_EKF2)
#include "ekf2/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ekf2::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_IKF)
#include "ikf/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::ikf::PoseEstimation, uquad::control::BlockLibrary)
#endif

#if defined(UQUAD_CONTROL_POSE_ESTIMATION_MADGWICK)
#include "madgwick/PoseEstimation.h"
UQUAD_BASE_REGISTER_OBJECT(uquad::control::madgwick::PoseEstimation, uquad::control::BlockLibrary)
#endif
