#include "Application.h"
#include <uquad/hal/DeviceManager.h>
#include <uquad/base/Error.h>

namespace cvcc
{
namespace remote
{

    Application::Application()
        : base::Application()
        , m_OptCameraName()
        , m_OptCameraWidth()
        , m_OptCameraHeight()
        , m_OptCameraRate()
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
            ("camera,c", po::value(&m_OptCameraName)->default_value(""), "specific camera to calibrate")
            ("width,w", po::value(&m_OptCameraWidth)->default_value(640), "camera capture width")
            ("height,h", po::value(&m_OptCameraHeight)->default_value(480), "camera capture height")
            ("rate,r", po::value(&m_OptCameraRate)->default_value(25), "camera capture rate");
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
        
        m_Camera->cameraDelegate() += this;
        
        m_Camera->beginConfiguration();
        m_Camera->setPreviewSize(Vec2i(m_OptCameraWidth, m_OptCameraHeight));
        if(!m_Camera->getSupportedVideoSizes().empty())
        {
            m_Camera->setVideoSize(Vec2i(m_OptCameraWidth, m_OptCameraHeight));
        }
        m_Camera->endConfiguration();
        
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
    }
    
} //namespace remote
} //namespace cvcc
