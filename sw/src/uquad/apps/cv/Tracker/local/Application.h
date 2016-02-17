#ifndef CVTRACKER_LOCAL_APPLICATION_H
#define CVTRACKER_LOCAL_APPLICATION_H

#include "Config.h"

#include <uquad/base/Runloop.h>
#include <uquad/comm/RemoteControlClient.h>
#include <uquad/comm/CameraClient.h>

namespace cvtracker
{
namespace local
{
    class MainWindow;

    class Application
        : public QApplication
    {
    public:
        Application(int &argc, char **argv);
        ~Application();
        
        static Application* instance();
        
        MainWindow* getMainWindow();
        
        comm::RemoteControlClient& getRemoteControl();
        comm::CameraClient& getCameraClient();
        
    private:
        MainWindow *m_pMainWindow;
        
        intrusive_ptr<base::Runloop> m_Runloop;
        uquad::thread m_RunloopThread;
        comm::RemoteControlClient m_RemoteControl;
        comm::CameraClient m_CameraClient;
    };

} //namespace local
} //namespace cvtracker


#endif //CVTRACKER_LOCAL_APPLICATION_H