#ifndef UQUAD_TOOL_MAIN_WINDOW_H
#define UQUAD_TOOL_MAIN_WINDOW_H

#include "Config.h"
#include "ui_MainWindow.h"


namespace uquad
{
namespace tool
{
    class MainWindow
        : public QMainWindow
        , protected Ui::MainWindow
    {
        Q_OBJECT
    protected:
    
    
    public:
        MainWindow();
        ~MainWindow();
        
    protected:
        void createActions();
        void createToolbar();
        
    private slots:
        void startUQuadDevice();
        void stopUQuadDevice();
        
    private:
        QToolBar *m_ToolbarMain;
        QAction *m_ActionStartUQuadDevice;
        QAction *m_ActionStopUQuadDevice;
    };
    
    
} //namespace tool
} //namespace uquad

#endif //UQUAD_TOOL_MAIN_WINDOW_H