#ifndef UQUAD_CONTROL_POSE_ESTIMATION_H
#define UQUAD_CONTROL_POSE_ESTIMATION_H

#include "Config.h"
#include "Block.h"
#include "Signal.h"

namespace uquad
{
namespace control
{
    class PoseEstimation
        : public virtual Block
    {
    public:
        class Registry
            : public base::Object::Registry
        {
        public:
            Registry(std::string const &n);
        };
        
        typedef float DT;
        typedef float Time;
        
        typedef Vec3f VelocityRate;
        typedef Vec3f RotationRate;
        typedef Vec3f MagneticField;
        typedef float Altitude;
        
        typedef Quaternionf Attitude;
        typedef Vec3f Position;
        typedef Vec3f Velocity;
        
        static std::string const& dTName();
        static std::string const& timeName();
        static std::string const& velocityRateName();
        static std::string const& rotationRateName();
        static std::string const& magneticFieldName();
        static std::string const& relativeAltitudeName();

        static std::string const& attitudeName();
        static std::string const& positionName();
        static std::string const& velocityName();
        static std::string const& earthMagneticFieldName();
        
        PoseEstimation();
        ~PoseEstimation();
        
    protected:
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_POSE_ESTIMATION_H
