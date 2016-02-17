#include "MainWindow.h"
#include "Application.h"


namespace ctrlpe
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
        
        addToolBar(m_ToolbarMain);
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
    }
    
    void MainWindow::onRemoteControlDisconnected()
    {
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
} //namespace ctrlpe