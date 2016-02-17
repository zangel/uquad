#include "MainWindow.h"
#include "Application.h"


namespace cvtracker
{
namespace local
{
    MainWindow::MainWindow()
        : QMainWindow()
    {
        setupUi(this);
        createActions();
        createToolbar();
        updateStartStopUI();
        Application::instance()->getRemoteControl().remoteControlDelegate()+=this;
    }
    
    MainWindow::~MainWindow()
    {
        Application::instance()->getRemoteControl().remoteControlDelegate()-=this;
    }
    
    void MainWindow::createActions()
    {
        m_ActionStartRemoteControl = new QAction(QIcon(":/icons/Start.png"), tr("&Start"), this);
        m_ActionStartRemoteControl->setStatusTip(tr("Start"));
        connect(m_ActionStartRemoteControl, SIGNAL(triggered()), this, SLOT(startRemoteControl()));
        
        m_ActionStopRemoteControl = new QAction(QIcon(":/icons/Stop.png"), tr("S&top"), this);
        m_ActionStopRemoteControl->setStatusTip(tr("Stop"));
        connect(m_ActionStopRemoteControl, SIGNAL(triggered()), this, SLOT(stopRemoteControl()));
    }
    
    void MainWindow::createToolbar()
    {
        m_ToolbarMain = new QToolBar(this);
        m_ToolbarMain->setObjectName("MainToolbar");
        m_ToolbarMain->setIconSize(QSize(32, 32));
        m_ToolbarMain->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_ToolbarMain->setMovable(false);
        m_ToolbarMain->setFloatable(false);
        m_ToolbarMain->addAction(m_ActionStartRemoteControl);
        m_ToolbarMain->addAction(m_ActionStopRemoteControl);
        //m_ToolbarMain->addAction(m_ActionFileSaveProject);
        
        //m_ToolbarMain->addSeparator();
        
        //m_ToolbarMain->addAction(m_ActionDebugRun);
        //m_ToolbarMain->addAction(m_ActionDebugStop);

        //m_ToolbarMain->addSeparator();
        
        //m_ToolbarMain->addSeparator();
        //m_ToolbarMain->addAction(m_ActionViewNavigator);
        //m_ToolbarMain->addAction(m_ActionViewConsole);
        //m_ToolbarMain->addAction(m_ActionViewAssistant);
        
        addToolBar(m_ToolbarMain);
        
        //setUnifiedTitleAndToolBarOnMac(true);
    }
    
    void MainWindow::updateStartStopUI()
    {
        m_ActionStartRemoteControl->setEnabled(!Application::instance()->getRemoteControl().isStarted());
        m_ActionStopRemoteControl->setEnabled(Application::instance()->getRemoteControl().isStarted());
    }
    
    void MainWindow::onRemoteControlStarted()
    {
        QMetaObject::invokeMethod(
            this,
            "updateStartStopUI",
            Qt::QueuedConnection
        );
    }
    
    void MainWindow::onRemoteControlStopped()
    {
        QMetaObject::invokeMethod(
            this,
            "updateStartStopUI",
            Qt::QueuedConnection
        );
    }
    
    void MainWindow::onRemoteControlConnected()
    {
        Application::instance()->getCameraClient().setRemoteAddress(
            Application::instance()->getRemoteControl().getRemoteAddress()
        );
        
        Application::instance()->getCameraClient().start([](system::error_code e)
        {
            if(e)
            {
                Application::instance()->getRemoteControl().stop();
            }
            else
            {
                Application::instance()->getCameraClient().connect();
            }
        });
    }
    
    void MainWindow::onRemoteControlDisconnected()
    {
        Application::instance()->getCameraClient().disconnect([](system::error_code e)
        {
            Application::instance()->getCameraClient().stop();
        });
    }
    
    void MainWindow::startRemoteControl()
    {
        Application::instance()->getRemoteControl().start();
    }
    
    void MainWindow::stopRemoteControl()
    {
        Application::instance()->getRemoteControl().stop();
    }

} //namespace local
} //namespace cvtracker
