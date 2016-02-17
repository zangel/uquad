#include "Application.h"
#include "MainWindow.h"


namespace ctrlpe
{
namespace local
{

    Application::Application(int &argc, char **argv)
        : QApplication(argc, argv)
        , m_Runloop(new base::Runloop())
        , m_RunloopThread()
        , m_RemoteControl(m_Runloop)
    {
        setApplicationDisplayName("PoseEstimation");
        setOrganizationName("uQuad");
        setOrganizationDomain("uquad.com");
        setApplicationName("PoseEstimation");
     
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
    
    intrusive_ptr<base::Runloop> Application::getRunloop()
    {
        return m_Runloop;
    }
    
    comm::RemoteControlClient& Application::getRemoteControl()
    {
        return m_RemoteControl;
    }
    
} //namespace local
} //namespace ctrlpe
