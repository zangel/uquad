#ifndef UQUAD_CV_POSE_TRACKER_H
#define UQUAD_CV_POSE_TRACKER_H

#include "Config.h"
#include "Algorithm.h"
#include "../base/PixelSampleBuffer.h"


namespace uquad
{
namespace cv
{
    class CameraCalibration;
    
    class PoseTracker
        : public Algorithm
    {
    public:
        
        class Registry
            : public Algorithm::Registry
        {
        public:
            Registry(std::string const &n);
            ~Registry();
        };
        
        PoseTracker();
        ~PoseTracker();
        
        virtual Vec2i getImageSize() const = 0;
        virtual system::error_code setImageSize(Vec2i const &is) = 0;
        
        virtual system::error_code setCalibration(CameraCalibration const &cc) = 0;
        virtual system::error_code getTrackingFeatures(std::vector<Vec2i> &features) const = 0;
        virtual system::error_code getTrackingTrails(std::vector< std::pair<Vec2i, Vec2i> > &trails) const = 0;
        virtual system::error_code getReferenceGrid(std::vector<Vec2i> &grid, std::size_t hs = 8) const = 0;
        virtual system::error_code track(intrusive_ptr<base::PixelSampleBuffer> psb) = 0;
    };

} //namespace cv
} //namespace uquad

#endif //UQUAD_CV_POSE_TRACKER_H
