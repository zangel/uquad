#ifndef CTRLPE_LOCAL_POSE_ESTIMATION_VIEW_H
#define CTRLPE_LOCAL_POSE_ESTIMATION_VIEW_H

#include "Config.h"
#include "ui_PoseEstimationView.h"

#include "../common/msg/UQuadSensorsData.h"
#include "../common/msg/UQuadMotorsPower.h"


namespace ctrlpe
{
namespace local
{
    class PoseView;
    class EllipsoidView;
        
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
        void onAccelerometerCalibrationReset();
        void onAccelerometerCalibrationSave();
        void onAccelerometerCalibrationLoad();
        
        void onGyroscopeCalibrationReset();
        void onGyroscopeCalibrationSave();
        void onGyroscopeCalibrationLoad();
        
    
    private:
        void setupControlSystem();
        void setupGraphs();
        void replotGraphs();
        void handleUQuadSensorsDataReceived(intrusive_ptr<common::msg::UQuadSensorsData> sd);
        void startCurrentUQuadSensorsDataPlayoutTimer();
        void handleCurrentUQuadSensorsDataPlayoutTimer(system::error_code ec);
        void handleUQuadSensorsData(uquad::hal::UQuadSensorsData const &d);
        
        
    private:
        control::System m_ControlSystem;
        uquad::control::blocks::UQuadSensors m_UQuadSensors;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::Barometer::Altitude > > m_BarometerAltitude;
        intrusive_ptr< control::blocks::PoseEstimation > m_PoseEstimation;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::PoseEstimation::Attitude > > m_PoseEstimationAttitude;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::PoseEstimation::Position > > m_PoseEstimationPosition;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::PoseEstimation::Velocity > > m_PoseEstimationVelocity;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::PoseEstimation::MagneticField > > m_PoseEstimationEarthMagneticField;
        control::blocks::ManualControlSimple m_ManualControlSimple;
        control::blocks::AttitudeControlSimple m_AttitudeControlSimple;
        control::blocks::PositionControlSimple m_PositionControlSimple;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::PositionControlSimple::Attitude > > m_PositionControlSimpleAttitudeSetpoint;
        control::blocks::QuadMotorsThrustSimple m_QuadMotorsThrustSimple;
        intrusive_ptr< control::TypedOutputSignal< control::blocks::QuadMotorsThrust::MotorsPower > > m_QuadMotorsThrustMotorsPower;
        
        fast_mutex m_UQuadSensorsDataGuard;
        std::list< intrusive_ptr<common::msg::UQuadSensorsData> > m_UQuadSensorsData;
        intrusive_ptr<common::msg::UQuadSensorsData> m_CurrentUQuadSensorsData;
        asio::steady_timer m_CurrentUQuadSensorsDataPlayoutTimer;
        base::TimeDuration m_CurrentUQuadSensorsDataSampleDuration;
        uint32_t m_CurrentUQuadSensorsDataSample;
        
        fast_mutex m_GraphsGuard;
        base::TimePoint m_SensorsFirstTS;
        base::TimePoint m_SensorsLastTS;
    };
    
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_POSE_ESTIMATION_VIEW_H