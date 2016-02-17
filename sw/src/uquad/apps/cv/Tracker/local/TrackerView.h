#ifndef CVTRACKER_LOCAL_TRACKER_VIEW_H
#define CVTRACKER_LOCAL_TRACKER_VIEW_H

#include "Config.h"
#include "ui_TrackerView.h"
#include <uquad/comm/RemoteControl.h>

namespace cvtracker
{
namespace local
{
    class TrackerView
        : public QWidget
        , protected Ui::TrackerView
        , protected comm::RemoteControlDelegate
    {
    private:
        Q_OBJECT
    protected:
    
    public:
        TrackerView(QMainWindow *mw);
        ~TrackerView();
        
    protected:
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        
        Q_INVOKABLE void updateTrackerUI();
        
    protected slots:
    
    };
    
} //namespace local
} //namespace cvtracker

#endif //CVTRACKER_LOCAL_TRACKER_VIEW_H