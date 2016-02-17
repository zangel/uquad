#include "PoseEstimation.h"

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