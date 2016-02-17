#include "MainWindow.h"
#include "Application.h"


namespace uquad
{
namespace tool
{
    MainWindow::MainWindow()
        : QMainWindow()
    {
        setupUi(this);
        createActions();
        createToolbar();
    }
    
    MainWindow::~MainWindow()
    {
    }
    
    void MainWindow::createActions()
    {
        m_ActionStartUQuadDevice = new QAction(QIcon(":/icons/Start.png"), tr("&Start"), this);
        m_ActionStartUQuadDevice->setStatusTip(tr("Start"));
        connect(m_ActionStartUQuadDevice, SIGNAL(triggered()), this, SLOT(startUQuadDevice()));
        m_ActionStartUQuadDevice->setEnabled(!Application::instance()->getUQuadDevice().isStarted());
        
        m_ActionStopUQuadDevice = new QAction(QIcon(":/icons/Stop.png"), tr("S&top"), this);
        m_ActionStopUQuadDevice->setStatusTip(tr("Stop"));
        connect(m_ActionStopUQuadDevice, SIGNAL(triggered()), this, SLOT(stopUQuadDevice()));
        m_ActionStopUQuadDevice->setEnabled(Application::instance()->getUQuadDevice().isStarted());
    }
    
    void MainWindow::createToolbar()
    {
        m_ToolbarMain = new QToolBar(this);
        m_ToolbarMain->setObjectName("MainToolbar");
        m_ToolbarMain->setIconSize(QSize(32, 32));
        m_ToolbarMain->setToolButtonStyle(Qt::ToolButtonIconOnly);
        m_ToolbarMain->setMovable(false);
        m_ToolbarMain->setFloatable(false);
        m_ToolbarMain->addAction(m_ActionStartUQuadDevice);
        m_ToolbarMain->addAction(m_ActionStopUQuadDevice);
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
        
        setUnifiedTitleAndToolBarOnMac(true);
    }
    
    void MainWindow::startUQuadDevice()
    {
        Application::instance()->getUQuadDevice().open();
        Application::instance()->getUQuadDevice().start();
        m_ActionStartUQuadDevice->setEnabled(!Application::instance()->getUQuadDevice().isStarted());
        m_ActionStopUQuadDevice->setEnabled(Application::instance()->getUQuadDevice().isStarted());
    }
    
    void MainWindow::stopUQuadDevice()
    {
        Application::instance()->getUQuadDevice().stop();
        m_ActionStartUQuadDevice->setEnabled(!Application::instance()->getUQuadDevice().isStarted());
        m_ActionStopUQuadDevice->setEnabled(Application::instance()->getUQuadDevice().isStarted());
        Application::instance()->getUQuadDevice().close();
    }
    
    
} //namespace tool
} //namespace uquad