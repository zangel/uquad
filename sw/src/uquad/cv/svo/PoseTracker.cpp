#include "PoseTracker.h"
#include "../../base/Error.h"
#include <vikit/atan_camera.h>
#include <vikit/pinhole_camera.h>
#include <svo/map.h>
#include <svo/frame.h>
#include <svo/feature.h>
#include <vikit/math_utils.h>
#include <vikit/vision.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <sophus/se3.hpp>

namespace uquad
{
namespace cv
{
namespace svo
{

    PoseTracker::Registry::Registry()
        : cv::PoseTracker::Registry("svo")
    {
    }
    
    intrusive_ptr<cv::Algorithm> PoseTracker::Registry::createAlgorithm() const
    {
        return intrusive_ptr<cv::Algorithm>(new PoseTracker());
    }
    
    PoseTracker::PoseTracker()
        : cv::PoseTracker()
        , m_bPrepared(false)
        , m_ImageSize()
        , m_CameraCalibration()
        , m_VKCamera(0)
        , m_SVOFrameHandler(0)
    {
    }

    PoseTracker::~PoseTracker()
    {
    }
    
    bool PoseTracker::isValid() const
    {
        return true;
    }
        
    Vec2i PoseTracker::getImageSize() const
    {
        return m_ImageSize;
    }
    
    system::error_code PoseTracker::setImageSize(Vec2i const &is)
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_ImageSize = is;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::prepare()
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        #if defined(UQUAD_PLATFORM_iOS)
        m_VKCamera = new ::vk::PinholeCamera(
            640, 480,
            541.1559899747522, 541.8752324126422,
            319.5, 239.5,
            0.1050757785901151,
            0.01277592337706157,
            0.002884623400518438,
            0.001135479539957487,
            -0.1106934521170374
        );
        #else
        m_VKCamera = new ::vk::ATANCamera(
            m_ImageSize[0], m_ImageSize[1],
            m_CameraCalibration.fx, m_CameraCalibration.fy,
            m_CameraCalibration.cx, m_CameraCalibration.cy,
            m_CameraCalibration.s
        );
        #endif
        
        m_SVOFrameHandler = new ::svo::FrameHandlerMono(m_VKCamera);
        m_SVOFrameHandler->start();
        
        m_bPrepared = true;
        return base::makeErrorCode(base::kENoError);
    }
    
    bool PoseTracker::isPrepared() const
    {
        return m_bPrepared;
    }
    
    void PoseTracker::unprepare()
    {
        if(!m_bPrepared)
            return;
        
        m_SVOFrameHandler->reset();
        delete m_SVOFrameHandler; m_SVOFrameHandler = 0;
        delete m_VKCamera; m_VKCamera = 0;
        
        m_bPrepared = false;
    }
    
    system::error_code PoseTracker::setCalibration(CameraCalibration const &cc)
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_CameraCalibration = cc;
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getTrackingFeatures(std::vector<Vec2i> &features) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        features.clear();
        if(m_SVOFrameHandler->lastFrame())
        {
            features.resize(m_SVOFrameHandler->lastFrame()->fts_.size());
        
            ::svo::Features::const_iterator iff = m_SVOFrameHandler->lastFrame()->fts_.begin();
        
            for(std::size_t c = 0; c < features.size(); ++c, ++iff)
                features[c] = Vec2i((*iff)->px[0], (*iff)->px[1]);
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getTrackingTrails(std::vector< std::pair<Vec2i, Vec2i> > &trails) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getReferenceGrid(std::vector<Vec2i> &grid, std::size_t hs) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::track(intrusive_ptr<base::PixelSampleBuffer> psb)
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        Vec2i bfferSize = psb->planeSize();
        
        ::cv::Mat gray(
            bfferSize[1],
            bfferSize[0],
            CV_8UC1,
            psb->planeData(),
            psb->planeRowBytes()
        );
        
        const double ts = chrono::duration_cast<chrono::nanoseconds>(psb->time.time_since_epoch()).count()*1.0e-9;
        m_SVOFrameHandler->addImage(gray, ts);
        
        return base::makeErrorCode(base::kENoError);
    }

} //namespace svo
} //namespace cv
} //namespace uquad
