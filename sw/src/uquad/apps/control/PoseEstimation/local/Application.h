#ifndef CTRLPE_LOCAL_APPLICATION_H
#define CTRLPE_LOCAL_APPLICATION_H

#include "Config.h"

#include <uquad/base/Runloop.h>
#include <uquad/comm/RemoteControlClient.h>

namespace ctrlpe
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
        
        intrusive_ptr<base::Runloop> getRunloop();
        comm::RemoteControlClient& getRemoteControl();

    private:
        MainWindow *m_pMainWindow;
        
        intrusive_ptr<base::Runloop> m_Runloop;
        uquad::thread m_RunloopThread;
        comm::RemoteControlClient m_RemoteControl;
    };

} //namespace local
} //namespace ctrlpe



#endif //CTRLPE_LOCAL_APPLICATION_H