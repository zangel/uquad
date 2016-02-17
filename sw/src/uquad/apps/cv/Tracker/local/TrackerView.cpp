#include "TrackerView.h"
#include "Application.h"
#include "../common/msg/TrackerInfo.h"
#include <fstream>

namespace cvtracker
{
namespace local
{
    TrackerView::TrackerView(QMainWindow *mw)
        : QWidget(mw)
        , Ui::TrackerView()
    {
        setupUi(this);
        Application::instance()->getRemoteControl().remoteControlDelegate() += this;
    }
    
    TrackerView::~TrackerView()
    {
        Application::instance()->getRemoteControl().remoteControlDelegate() -= this;
    }
    
    void TrackerView::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
    }
    
    void TrackerView::updateTrackerUI()
    {
    }


} //namespace local
} //namespace cvtracker

