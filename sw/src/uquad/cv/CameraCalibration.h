#ifndef UQUAD_CV_CAMERA_CALIBRATION_H
#define UQUAD_CV_CAMERA_CALIBRATION_H

#include "Config.h"
#include "../base/Serialization.h"

namespace uquad
{
namespace cv
{
    class CameraCalibration
    {
    public:
        CameraCalibration();
        
    public:
        friend class boost::serialization::access;
        
        template<class Archive>
        void serialize(Archive &ar, const unsigned int) BOOST_USED;
        
        double fx;
        double fy;
        double cx;
        double cy;
        double s;
    };
    
} //namespace cv
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT(uquad::cv::CameraCalibration)

#endif //UQUAD_CV_CAMERA_CALIBRATION_H



