#include "PoseTracker.h"
#include "../../base/Error.h"
#include <cvd/image.h>

namespace uquad
{
namespace cv
{
namespace ptam
{
    PoseTracker::Registry::Registry()
        : cv::PoseTracker::Registry("ptam")
    {
    }
    
    intrusive_ptr<cv::Algorithm> PoseTracker::Registry::createAlgorithm() const
    {
        return intrusive_ptr<cv::Algorithm>(new PoseTracker());
    }
    
    PoseTracker::PoseTracker()
        : cv::PoseTracker()
        , m_Context()
        , m_Camera(&m_Context, "Camera")
        , m_Map()
        , m_MapMaker(&m_Context, m_Map, m_Camera)
        , m_Tracker(&m_Context, m_Camera, m_Map, m_MapMaker)
        , m_bPrepared(false)
    {
        m_Context.ptamTrackerAutoInit = true;
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
        ::Vector<2> camSize = m_Camera.GetImageSize();
        return Vec2i(camSize[0], camSize[1]);
    }
    
    system::error_code PoseTracker::setImageSize(Vec2i const &is)
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_Camera.SetImageSize(::makeVector(is[0], is[1]));
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::prepare()
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
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
        
        m_bPrepared = false;
    }
    
    system::error_code PoseTracker::setCalibration(CameraCalibration const &cc)
    {
        if(m_bPrepared)
            return base::makeErrorCode(base::kEAlreadyStarted);
        
        m_Context.calibration = cc;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getTrackingFeatures(std::vector<Vec2i> &features) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        std::vector<CVD::ImageRef> const& ptamCorners = m_Tracker.GetTrackingCorners();
        
        features.clear();
        features.resize(ptamCorners.size());
        for(std::size_t c = 0; c < ptamCorners.size(); ++c)
            features[c] = Vec2i(ptamCorners[c].x, ptamCorners[c].y);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getTrackingTrails(std::vector< std::pair<Vec2i, Vec2i> > &trails) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        if(!m_Map.IsGood())
        {
            std::list<Trail> const &ptamTrails = m_Tracker.GetTrackingTrails();
            trails.clear();
            trails.resize(ptamTrails.size());
            std::size_t iit = 0;
            for(std::list<Trail>::const_iterator it = ptamTrails.begin(); it != ptamTrails.end(); ++it, ++iit)
            {
                trails[iit] = std::make_pair(
                    Vec2i(it->irInitialPos.x, it->irInitialPos.y),
                    Vec2i(it->irCurrentPos.x, it->irCurrentPos.y)
                );
            }
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::getReferenceGrid(std::vector<Vec2i> &grid, std::size_t hs) const
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        if(!m_Map.IsGood())
            return base::makeErrorCode(base::kENotFound);
        
        int const ihs = hs;
        int const nTot = ihs * 2 + 1;
        
        SE3<> mse3CamFromWorld = m_Tracker.GetCurrentPose();
        
        grid.resize(nTot*nTot);
        
        for(int i = 0; i < nTot; i++)
        {
            for(int j = 0; j < nTot; j++)
            {
                Vector<3> v3;
                v3[0] = (i - ihs) * 0.1;
                v3[1] = (j - ihs) * 0.1;
                v3[2] = 0.0;
                Vector<3> v3Cam = mse3CamFromWorld * v3;
                if(v3Cam[2] < 0.001)
                    v3Cam[2] = 0.001;
                Vector<2> projected = m_Camera.Project(project(v3Cam));
                grid[i*nTot+j] = Vec2i(projected[0], projected[1]);
            }
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code PoseTracker::track(intrusive_ptr<base::PixelSampleBuffer> psb)
    {
        if(!m_bPrepared)
            return base::makeErrorCode(base::kENotStarted);
        
        CVD::BasicImage<CVD::byte> cvdImg(
            reinterpret_cast<CVD::byte*>(psb->planeData()),
            CVD::ImageRef(
                psb->planeSize()[0],
                psb->planeSize()[1]
            ),
            psb->planeRowBytes()
        );
        
        m_Tracker.TrackFrame(cvdImg, false);
        
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace ptam
} //namespace cv
} //namespace uquad