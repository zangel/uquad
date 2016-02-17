#ifndef CVTRACKER_LOCAL_MAIN_WINDOW_H
#define CVTRACKER_LOCAL_MAIN_WINDOW_H

#include "Config.h"
#include "ui_MainWindow.h"
#include <uquad/comm/RemoteControl.h>


namespace cvtracker
{
namespace local
{
    class MainWindow
        : public QMainWindow
        , protected Ui::MainWindow
        , protected comm::RemoteControlDelegate
    {
        Q_OBJECT
    protected:
    
    
    public:
        MainWindow();
        ~MainWindow();
        
    protected:
        void createActions();
        void createToolbar();
        Q_INVOKABLE void updateStartStopUI();
        
        //RemoteControlDelegate
        void onRemoteControlStarted();
        void onRemoteControlStopped();
        void onRemoteControlConnected();
        void onRemoteControlDisconnected();
        
    private slots:
        void startRemoteControl();
        void stopRemoteControl();
        
    private:
        QToolBar *m_ToolbarMain;
        QAction *m_ActionStartRemoteControl;
        QAction *m_ActionStopRemoteControl;
    };

} //namespace local
} //namespace cvtracker

#endif //CVTRACKER_LOCAL_MAIN_WINDOW_H