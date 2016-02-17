#include "Application.h"
#include <uquad/hal/DeviceManager.h>
#include <uquad/cv/AlgorithmLibrary.h>
#include <uquad/base/Error.h>
#include "../common/msg/TrackerInfo.h"
#include <fstream>

#define CVTRACKER_DEFAULT_TRACKER "ptam"
#if defined(UQUAD_PLATFORM_Android)
#define CVTRACKER_DEFAULT_CAMERA "Camera1"
#else
#define CVTRACKER_DEFAULT_CAMERA ""
#endif

namespace cvtracker
{
namespace remote
{

    Application::Application()
        : base::Application()
        , m_OptCameraName()
        , m_OptCameraWidth()
        , m_OptCameraHeight()
        , m_OptCameraRate()
        , m_OptTracker()
        , m_Camera()
        , m_CameraServer(mainRunloop())
        , m_RemoteControlServer(mainRunloop())
    {
        m_RemoteControlServer.remoteControlDelegate() += this;
    }
    
    Application::~Application()
    {
        m_RemoteControlServer.remoteControlDelegate() -= this;
    }
    
    
    void Application::buildOptions(po::options_description &desc)
    {
        desc.add_options()
            ("camera,c", po::value(&m_OptCameraName)->default_value(CVTRACKER_DEFAULT_CAMERA), "specific camera to calibrate")
            ("width,w", po::value(&m_OptCameraWidth)->default_value(320), "camera capture width")
            ("height,h", po::value(&m_OptCameraHeight)->default_value(240), "camera capture height")
            ("rate,r", po::value(&m_OptCameraRate)->default_value(25), "camera capture rate")
            ("tracker,t", po::value(&m_OptTracker)->default_value(CVTRACKER_DEFAULT_TRACKER), "tracker to use");
    }
    
    system::error_code Application::prepare()
    {
        if(m_OptCameraName.empty())
        {
            SI(hal::DeviceManager).enumerateDevices<hal::Camera>([this](intrusive_ptr<hal::Camera> const &c) -> bool
            {
                m_Camera = c;
                return false;
            });
        }
        else
        {
            m_Camera = SI(hal::DeviceManager).findDevice<hal::Camera>(m_OptCameraName);
        }
        
        if(!m_Camera || m_Camera->open())
            return base::makeErrorCode(base::kENoSuchDevice);
        
        
        
        m_Camera->beginConfiguration();
        m_Camera->setPreviewSize(Vec2i(m_OptCameraWidth, m_OptCameraHeight));
        if(!m_Camera->getSupportedVideoSizes().empty())
        {
            m_Camera->setVideoSize(Vec2i(m_OptCameraWidth, m_OptCameraHeight));
        }
        m_Camera->endConfiguration();
        
        m_PoseTracker = SI(cv::AlgorithmLibrary).createPoseTracker(m_OptTracker);
        m_PoseTracker->setImageSize(m_Camera->getPreviewSize());
        if(loadCameraCalibration(dataDirectory()/(std::string(m_Camera->name() + ".ccal"))))
        {
            m_PoseTracker->setCalibration(m_CameraCalibration);
            //m_Camera->close();
            //m_Camera.reset();
            //return base::makeErrorCode(base::kENotFound);
        }

        if(m_PoseTracker->prepare())
        {
            m_Camera->close();
            m_Camera.reset();
            m_PoseTracker.reset();
            return base::makeErrorCode(base::kENotFound);
        }
        
        m_Camera->cameraDelegate() += this;
        
        m_Camera->startPreview();
        m_Camera->startRecording();

        m_CameraServer.setCamera(m_Camera);
        m_CameraServer.start();
        m_RemoteControlServer.start();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void Application::cleanup()
    {
        m_Camera->stopRecording();
        m_Camera->stopPreview();
        m_Camera->close();
        m_Camera->cameraDelegate() -= this;
        m_Camera.reset();
    }
    
    void Application::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
    }
    
    void Application::onCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        /*
        static bool tested = false;
        if(!tested)
        {
            intrusive_ptr<common::msg::TrackerInfo> msgTrackerInfo(new common::msg::TrackerInfo());
            #if 0
            msgTrackerInfo->trackingFeatures.resize(20000, Vec2i(100,100));
            msgTrackerInfo->trackingTrails.resize(100, std::make_pair(Vec2i(100,100), Vec2i(200,200)));
            msgTrackerInfo->referenceGrid.resize(17*17, Vec2i(10,10));
            msgTrackerInfo->referenceGridHalfSize = 8;

            {
                std::ofstream file("TrackerInfo.msg");
                base::OArchive archive(file);
                archive & msgTrackerInfo;
            }
            #else
            {
                std::ifstream file("TrackerInfo.msg");
                base::IArchive archive(file);
                archive & msgTrackerInfo;
            }
            #endif
            tested = true;
        }
        */

        if(buffer->lock())
            return;
        m_PoseTracker->track(buffer);
        buffer->unlock();
        
        if(m_RemoteControlServer.isConnected())
        {
            intrusive_ptr<common::msg::TrackerInfo> msgTrackerInfo(new common::msg::TrackerInfo());
            m_PoseTracker->getTrackingFeatures(msgTrackerInfo->trackingFeatures);
            m_PoseTracker->getTrackingTrails(msgTrackerInfo->trackingTrails);
            m_PoseTracker->getReferenceGrid(msgTrackerInfo->referenceGrid, 8);
            msgTrackerInfo->referenceGridHalfSize = 8;
            m_RemoteControlServer.sendMessage(msgTrackerInfo);

            
            static int c = 1;
            if(!--c)
            {
                std::cout << "trackingFeatures.size=" << msgTrackerInfo->trackingFeatures.size() << std::endl;
                std::cout << "trackingTrails.size=" << msgTrackerInfo->trackingTrails.size() << std::endl;
                c = 30;
            }
        }
    }
    
    bool Application::loadCameraCalibration(fs::path const &ccPath)
    {
        {
            std::ifstream ccFile(ccPath.string());
            if(!ccFile.is_open())
                return false;
            
            cv::CameraCalibration &ccal = m_CameraCalibration;
            {
                base::IArchive ccArchive(ccFile);
                ccArchive & ccal;
            }
            
        }
        
        return true;
    }
    
} //namespace remote
} //namespace cvtracker
