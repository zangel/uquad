#include "CameraCalibration.h"

namespace uquad
{
namespace cv
{
    CameraCalibration::CameraCalibration()
        : fx(0.5)
        , fy(0.75)
        , cx(0.5)
        , cy(0.5)
        , s(0.1)
    {
    }
    
    template<class Archive>
    void CameraCalibration::serialize(Archive &ar, const unsigned int)
    {
        ar & fx;
        ar & fy;
        ar & cx;
        ar & cy;
        ar & s;
    }
    

} //namespace cv
} //namespace uquad

UQUAD_BASE_SERIALIZATION_EXPORT_IMPLEMENT(uquad::cv::CameraCalibration)
