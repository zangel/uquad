#ifndef UQUAD_CONTROL_POSE_ESTIMATION_H
#define UQUAD_CONTROL_POSE_ESTIMATION_H

#include "Config.h"
#include "System.h"
#include "../hal/UQuad.h"

namespace uquad
{
namespace control
{
    class PoseEstimation
        : public System
    {
    public:
    
        class Registry
            : public base::Object::Registry
        {
        public:
            Registry(std::string const &n);
        };
        
        PoseEstimation();
        ~PoseEstimation();
        
        
        virtual system::error_code processUQuadSensorsData(hal::UQuadSensorsData const &usd) = 0;
        
        Quaternionf attitude;
        Vec3f velocity;
        Vec3f position;
        
        float velocityRateZBias;
        Vec3f roationRateBias;
        Vec2f wind;
        
        Vec3f earthMagneticField;
        Vec3f bodyMagneticField;
        
    protected:
    };
    
} //namespace control
} //namespace uquad

#endif //UQUAD_CONTROL_POSE_ESTIMATION_H
