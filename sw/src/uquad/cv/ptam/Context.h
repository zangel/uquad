#ifndef UQUAD_CV_PTAM_CONTEXT_H
#define UQUAD_CV_PTAM_CONTEXT_H

#include "../Config.h"
#include <ptam/Context.h>
#include "../CameraCalibration.h"

namespace uquad
{
namespace cv
{
namespace ptam
{
    class Context
        : public ::Context
    {
    public:
        Context();
        ~Context();
        
        CameraCalibration calibration;
    };

} //namespace ptam
} //namespace cv
} //namespace uquad

#endif //UQUAD_CV_PTAM_CONTEXT_H
