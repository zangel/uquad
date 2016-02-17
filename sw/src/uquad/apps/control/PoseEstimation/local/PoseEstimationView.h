#ifndef CTRLPE_LOCAL_POSEESTIMATION_VIEW_H
#define CTRLPE_LOCAL_POSEESTIMATION_VIEW_H

#include "Config.h"
#include "ui_PoseEstimationView.h"
#include <uquad/comm/RemoteControl.h>
#include "../common/msg/UQuadSensorsData.h"
#include <uquad/control/PoseEstimation.h>

namespace ctrlpe
{
namespace local
{
    class PoseEstimationView
        : public QWidget
        , protected Ui::PoseEstimationView
        , protected comm::RemoteControlDelegate
    {
    private:
        
        Q_OBJECT
    protected:
    
    public:
        PoseEstimationView(QMainWindow *mw);
        ~PoseEstimationView();
        
    protected:
        void onRemoteControlStarted();
        void onRemoteControlStopped();
        
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        
        Q_INVOKABLE void updatePoseEstimationUI();
        
        
    protected slots:
    
    private:
        void setupGraphs();
        Q_INVOKABLE void replotGraphs();
        void handleUQuadSensorsDataReceived(intrusive_ptr<common::msg::UQuadSensorsData> sd);
        void startCurrentUQuadSensorsDataPlayoutTimer();
        void handleCurrentUQuadSensorsDataPlayoutTimer(system::error_code ec);
        void handleUQuadSensorsData(uquad::hal::UQuadSensorsData const &d);
        
        
    private:
        intrusive_ptr<control::PoseEstimation> m_PoseEstimation;
        
        fast_mutex m_UQuadSensorsDataGuard;
        std::list< intrusive_ptr<common::msg::UQuadSensorsData> > m_UQuadSensorsData;
        intrusive_ptr<common::msg::UQuadSensorsData> m_CurrentUQuadSensorsData;
        asio::steady_timer m_CurrentUQuadSensorsDataPlayoutTimer;
        base::TimeDuration m_CurrentUQuadSensorsDataSampleDuration;
        uint32_t m_CurrentUQuadSensorsDataSample;
        
        fast_mutex m_GraphsGuard;
        base::TimePoint m_SensorsFirstTS;
    };
    
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_POSEESTIMATION_VIEW_H