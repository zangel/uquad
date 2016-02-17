#ifndef CVTRACKER_REMOTE_APPLICATION_H
#define CVTRACKER_REMOTE_APPLICATION_H

#include <uquad/Config.h>
#include <uquad/base/Application.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/Camera.h>
#include <uquad/comm/CameraServer.h>
#include <uquad/comm/RemoteControlServer.h>
#include <uquad/cv/CameraCalibration.h>
#include <uquad/cv/PoseTracker.h>


namespace cvtracker
{
namespace remote
{
    using namespace uquad;
    
    class Application
        : public base::Application
        , protected comm::RemoteControlDelegate
        , protected hal::CameraDelegate
    {
    public:
        Application();
        ~Application();
        
        void buildOptions(po::options_description &desc);
        system::error_code prepare();
        void cleanup();
        
    protected:
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        void onCameraPreviewFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
    private:
        bool loadCameraCalibration(fs::path const &ccal);
        
    private:
        std::string m_OptCameraName;
        int m_OptCameraWidth;
        int m_OptCameraHeight;
        int m_OptCameraRate;
        std::string m_OptTracker;
        
        intrusive_ptr<hal::Camera> m_Camera;
        comm::CameraServer m_CameraServer;
        comm::RemoteControlServer m_RemoteControlServer;
        cv::CameraCalibration m_CameraCalibration;
        intrusive_ptr<cv::PoseTracker> m_PoseTracker;
    };
    
} //namespace remote
} //namespace cvtracker


#endif //CVCC_REMOTE_APPLICATION_H