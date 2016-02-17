#ifndef UQUAD_CV_SVO_POSE_TRACKER_H
#define UQUAD_CV_SVO_POSE_TRACKER_H

#include "../PoseTracker.h"
#include "../CameraCalibration.h"

#include <svo/config.h>
#include <svo/frame_handler_mono.h>
#include <vikit/abstract_camera.h>


namespace uquad
{
namespace cv
{
namespace svo
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
        bool m_bPrepared;
        Vec2i m_ImageSize;
        CameraCalibration m_CameraCalibration;
        ::vk::AbstractCamera *m_VKCamera;
        ::svo::FrameHandlerMono *m_SVOFrameHandler;
    };

} //namespace svo
} //namespace cv
} //namespace uquad


#endif //UQUAD_CV_SVO_POSE_TRACKER_H
