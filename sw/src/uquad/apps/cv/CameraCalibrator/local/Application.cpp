#include "Application.h"
#include "MainWindow.h"


namespace cvcc
{
namespace local
{

    Application::Application(int &argc, char **argv)
        : QApplication(argc, argv)
        , m_Runloop(new base::Runloop())
        , m_RunloopThread()
        , m_RemoteControl(m_Runloop)
        , m_CameraClient(m_Runloop)
    {
        setApplicationDisplayName("CameraCalibrator");
        setOrganizationName("uQuad");
        setOrganizationDomain("uquad.com");
        setApplicationName("CameraCalibrator");
     
        m_pMainWindow = new MainWindow();
        m_pMainWindow->show();
        
        m_RunloopThread = std::move(uquad::thread([this]() { m_Runloop->run(); }));
    }
    
    Application::~Application()
    {
        m_Runloop->stop();
        m_RunloopThread.join();
    }
    
    Application* Application::instance()
    {
        return static_cast<Application*>(QCoreApplication::instance());
    }
    
    MainWindow* Application::getMainWindow()
    {
        return m_pMainWindow;
    }
    
    comm::RemoteControlClient& Application::getRemoteControl()
    {
        return m_RemoteControl;
    }
    
    comm::CameraClient& Application::getCameraClient()
    {
        return m_CameraClient;
    }
    
} //namespace local
} //namespace cvcc
