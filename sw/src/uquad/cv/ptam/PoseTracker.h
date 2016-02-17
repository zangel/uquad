#ifndef UQUAD_CV_PTAM_POSE_TRACKER_H
#define UQUAD_CV_PTAM_POSE_TRACKER_H

#include "../PoseTracker.h"
#include "Context.h"

#include <ptam/Map.h>
#include <ptam/MapMaker.h>
#include <ptam/Tracker.h>
#include <ptam/ATANCamera.h>


namespace uquad
{
namespace cv
{
namespace ptam
{
    class PoseTracker
        : public cv::PoseTracker
    {
    public:
        class Registry
			: public cv::PoseTracker::Registry
		{
		public:
			Registry();

			intrusive_ptr<cv::Algorithm> createAlgorithm() const;
		};
        
        PoseTracker();
        ~PoseTracker();
        
        bool isValid() const;
        
        Vec2i getImageSize() const;
        system::error_code setImageSize(Vec2i const &is);
        
        system::error_code prepare();
        bool isPrepared() const;
        void unprepare();
        
        system::error_code setCalibration(CameraCalibration const &cc);
        system::error_code getTrackingFeatures(std::vector<Vec2i> &features) const;
        system::error_code getTrackingTrails(std::vector< std::pair<Vec2i, Vec2i> > &trails) const;
        system::error_code getReferenceGrid(std::vector<Vec2i> &grid, std::size_t hs = 8) const;
        system::error_code track(intrusive_ptr<base::PixelSampleBuffer> psb);
    protected:
        
    private:
        Context m_Context;
        mutable ::ATANCamera m_Camera;
        ::Map m_Map;
        ::MapMaker m_MapMaker;
        ::Tracker m_Tracker;
        bool m_bPrepared;
    };

} //namespace ptam
} //namespace cv
} //namespace uquad

#endif //UQUAD_CV_PTAM_POSE_TRACKER_H
