#include "Application.h"
#include "MainWindow.h"


namespace uquad
{
namespace tool
{

    Application::Application(int &argc, char **argv)
        : QApplication(argc, argv)
    {
        setApplicationDisplayName("uQuadTool");
        setOrganizationName("uQuad");
        setOrganizationDomain("uquad.com");
        setApplicationName("uQuadTool");
     
        m_pMainWindow = new MainWindow();
        m_pMainWindow->show();
    }
    
    Application::~Application()
    {
    }
    
    Application* Application::instance()
    {
        return static_cast<Application*>(QCoreApplication::instance());
    }
    
    MainWindow* Application::getMainWindow()
    {
        return m_pMainWindow;
    }
    
    hal::UQuadDevice& Application::getUQuadDevice()
    {
        return m_UQuadDevice;
    }

} //namespace tool
} //namespace uquad
