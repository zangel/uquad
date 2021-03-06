#include "PoseEstimationView.h"
#include "Application.h"
#include "PoseView.h"
#include "CalibrationView.h"
#include <uquad/control/BlockLibrary.h>
#include <uquad/math/Utils.h>
#include <uquad/base/Serialization.h>
#include <fstream>

namespace ctrlpe
{
namespace local
{
    template <typename T>
    T wrapPi(T x) { return x<0 ? x+2*M_PI:(x>2*M_PI?x-2*M_PI:x); }

    PoseEstimationView::PoseEstimationView(QMainWindow *mw)
        : QWidget(mw)
        , Ui::PoseEstimationView()
        , m_ControlSystem(Application::instance()->getRunloop())
        , m_UQuadSensors()
        , m_BarometerAltitude()
        , m_PoseEstimation(SI(control::BlockLibrary).createPoseEstimation("ekf2"))
        , m_PoseEstimationAttitude()
        , m_PoseEstimationPosition()
        , m_PoseEstimationEarthMagneticField()
        , m_ManualControlSimple()
        , m_AttitudeControlSimple()
        , m_PositionControlSimple()
        , m_QuadMotorsThrustSimple()
        , m_QuadMotorsThrustMotorsPower()
        , m_UQuadSensorsDataGuard()
        , m_UQuadSensorsData()
        , m_CurrentUQuadSensorsData()
        , m_CurrentUQuadSensorsDataPlayoutTimer(Application::instance()->getRunloop()->ios())
        , m_CurrentUQuadSensorsDataSampleDuration()
        , m_CurrentUQuadSensorsDataSample(0)
    {
        
        intrusive_ptr_add_ref(&m_ControlSystem);
        intrusive_ptr_add_ref(&m_UQuadSensors);
        
        m_BarometerAltitude = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::Barometer::Altitude >
        >(m_UQuadSensors.outputSignalAltitude());
        
        m_PoseEstimationAttitude = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::PoseEstimation::Attitude >
        >(m_PoseEstimation->outputSignalAttitude());
        
        m_PoseEstimationPosition = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::PoseEstimation::Position >
        >(m_PoseEstimation->outputSignalPosition());
        
        m_PoseEstimationVelocity = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::PoseEstimation::Velocity >
        >(m_PoseEstimation->outputSignalVelocity());
        
        m_PoseEstimationEarthMagneticField = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::PoseEstimation::MagneticField >
        >(m_PoseEstimation->outputSignalEarthMagneticField());
        
        intrusive_ptr_add_ref(&m_ManualControlSimple);
        
        intrusive_ptr_add_ref(&m_AttitudeControlSimple);
        
        intrusive_ptr_add_ref(&m_PositionControlSimple);
        
        m_PositionControlSimpleAttitudeSetpoint = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::PositionControlSimple::Attitude >
        >(m_PositionControlSimple.outputSignalAttitudeSetpoint());
        
        intrusive_ptr_add_ref(&m_QuadMotorsThrustSimple);
        
        m_QuadMotorsThrustMotorsPower = dynamic_pointer_cast
        <
            control::TypedOutputSignal< control::blocks::QuadMotorsThrust::MotorsPower >
        >(m_QuadMotorsThrustSimple.outputSignalMotorsPower());
        
        setupControlSystem();
        
        setupUi(this);
        setupGraphs();
        Application::instance()->getRemoteControl().remoteControlDelegate() += this;
        
        connect(
            m_AccelerometerCalibrationReset, SIGNAL(clicked()),
            this, SLOT(onAccelerometerCalibrationReset())
        );
        
        connect(
            m_AccelerometerCalibrationSave, SIGNAL(clicked()),
            this, SLOT(onAccelerometerCalibrationSave())
        );
        
        connect(
            m_AccelerometerCalibrationLoad, SIGNAL(clicked()),
            this, SLOT(onAccelerometerCalibrationLoad())
        );
        
        
        connect(
            m_GyroscopeCalibrationReset, SIGNAL(clicked()),
            this, SLOT(onGyroscopeCalibrationReset())
        );
        
        connect(
            m_GyroscopeCalibrationSave, SIGNAL(clicked()),
            this, SLOT(onGyroscopeCalibrationSave())
        );
        
        connect(
            m_GyroscopeCalibrationLoad, SIGNAL(clicked()),
            this, SLOT(onGyroscopeCalibrationLoad())
        );
        
        updatePoseEstimationUI();
        
    }
    
    PoseEstimationView::~PoseEstimationView()
    {
        Application::instance()->getRemoteControl().remoteControlDelegate() -= this;
    }
    
    void PoseEstimationView::onRemoteControlStarted()
    {
        m_ControlSystem.start();
        
        lock_guard<fast_mutex> graphsGuard(m_GraphsGuard);
        
        m_VelocityRate->graph(0)->clearData();
        m_VelocityRate->graph(1)->clearData();
        m_VelocityRate->graph(2)->clearData();
            
        m_RotationRate->graph(0)->clearData();
        m_RotationRate->graph(1)->clearData();
        m_RotationRate->graph(2)->clearData();
            
        m_MagneticField->graph(0)->clearData();
        m_MagneticField->graph(1)->clearData();
        m_MagneticField->graph(2)->clearData();
            
        m_Barometrics->graph(0)->clearData();
        
        m_AttitudeEst->graph(0)->clearData();
        m_AttitudeEst->graph(1)->clearData();
        m_AttitudeEst->graph(2)->clearData();
        
        m_VelocityEst->graph(0)->clearData();
        m_VelocityEst->graph(1)->clearData();
        m_VelocityEst->graph(2)->clearData();
        
        m_PositionEst->graph(0)->clearData();
        m_PositionEst->graph(1)->clearData();
        m_PositionEst->graph(2)->clearData();
        
        m_EarthMagneticFieldEst->graph(0)->clearData();
        m_EarthMagneticFieldEst->graph(1)->clearData();
        m_EarthMagneticFieldEst->graph(2)->clearData();
        
        m_UQuadSensorsData.clear();
        m_CurrentUQuadSensorsData.reset();
        m_SensorsFirstTS = base::TimePoint::min();
    }
    
    void PoseEstimationView::onRemoteControlStopped()
    {
        m_CurrentUQuadSensorsDataPlayoutTimer.cancel();
        m_ControlSystem.stop();
    }
    
    void PoseEstimationView::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
        if(!Application::instance()->getRemoteControl().isStarted())
            return;
        
        if(intrusive_ptr<common::msg::UQuadSensorsData> sensorsData = dynamic_pointer_cast<common::msg::UQuadSensorsData>(msg))
        {
            handleUQuadSensorsDataReceived(sensorsData);
        }
    }
    
    void PoseEstimationView::updatePoseEstimationUI()
    {
        replotGraphs();
        m_AccelerometerCalibrationReset->setEnabled(m_CalibrationView->isAccelerometerCalibrated());
        m_AccelerometerCalibrationSave->setEnabled(m_CalibrationView->isAccelerometerCalibrated());
        
        m_GyroscopeCalibrationReset->setEnabled(m_CalibrationView->isGyroscopeCalibrated());
        m_GyroscopeCalibrationSave->setEnabled(m_CalibrationView->isGyroscopeCalibrated());
    }
    
    void PoseEstimationView::onAccelerometerCalibrationReset()
    {
        m_CalibrationView->resetAccelerometerCalibration();
        updatePoseEstimationUI();
    }
    
    
    void PoseEstimationView::onAccelerometerCalibrationSave()
    {
        QString calibFileName = QFileDialog::getSaveFileName(this, tr("Save calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ofstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::OArchive ccArchive(ccFile);
            ccArchive & m_CalibrationView->accelerometerCalibration();
        }
    }
    
    void PoseEstimationView::onAccelerometerCalibrationLoad()
    {
        QString calibFileName = QFileDialog::getOpenFileName(this, tr("Load calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ifstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::IArchive ccArchive(ccFile);
            control::AccelerometerCalibration calibration;
            ccArchive & calibration;
            
            m_UQuadSensors.setAccelerometerCalibration(calibration.calibrationTransform());
        }
    }
    
    void PoseEstimationView::onGyroscopeCalibrationReset()
    {
        m_CalibrationView->resetGyroscopeCalibration();
        updatePoseEstimationUI();
    }
    
    
    void PoseEstimationView::onGyroscopeCalibrationSave()
    {
        QString calibFileName = QFileDialog::getSaveFileName(this, tr("Save calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ofstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::OArchive ccArchive(ccFile);
            ccArchive & m_CalibrationView->gyroscopeCalibration();
        }
    }
    
    void PoseEstimationView::onGyroscopeCalibrationLoad()
    {
        QString calibFileName = QFileDialog::getOpenFileName(this, tr("Load calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ifstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::IArchive ccArchive(ccFile);
            control::GyroscopeCalibration calibration;
            ccArchive & calibration;
            
            m_UQuadSensors.setGyroscopeCalibration(calibration.calibrationTransform());
        }
    }
    
    void PoseEstimationView::setupControlSystem()
    {
        m_ControlSystem.addBlock(&m_UQuadSensors);              //0
        m_ControlSystem.addBlock(m_PoseEstimation);             //1
        m_ControlSystem.addBlock(&m_ManualControlSimple);       //2
        m_ControlSystem.addBlock(&m_AttitudeControlSimple);     //3
        m_ControlSystem.addBlock(&m_PositionControlSimple);     //4
        m_ControlSystem.addBlock(&m_QuadMotorsThrustSimple);    //5
        
        
        // UQuadSensors -> PoseEstimation
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalTime(), m_PoseEstimation->inputSignalTime());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalDT(), m_PoseEstimation->inputSignalDT());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalVelocityRate(), m_PoseEstimation->inputSignalVelocityRate());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalRotationRate(), m_PoseEstimation->inputSignalRotationRate());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalMagneticField(), m_PoseEstimation->inputSignalMagneticField());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalAltitude(), m_PoseEstimation->inputSignalAltitude());
        
        // ManualControlSimple -> PositionControlSimple
        m_ControlSystem.connectPorts(m_ManualControlSimple.outputSignalDirection(), m_PositionControlSimple.inputSignalDirectionControl());
        m_ControlSystem.connectPorts(m_ManualControlSimple.outputSignalThrust(), m_PositionControlSimple.inputSignalThrustControl());
        
        
        // UQuadSensors -> PositionControlSimple
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalDT(), m_PositionControlSimple.inputSignalDT());
        
        // PoseEstimation -> PositionControlSimple
        m_ControlSystem.connectPorts(m_PoseEstimation->outputSignalAttitude(), m_PositionControlSimple.inputSignalAttitudeEstimated());
        m_ControlSystem.connectPorts(m_PoseEstimation->outputSignalPosition(), m_PositionControlSimple.inputSignalPositionEstimated());
        m_ControlSystem.connectPorts(m_PoseEstimation->outputSignalVelocity(), m_PositionControlSimple.inputSignalVelocityEstimated());
        
        // UQuadSensors -> AttitudeControlSimple
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalDT(), m_AttitudeControlSimple.inputSignalDT());
        m_ControlSystem.connectPorts(m_UQuadSensors.outputSignalRotationRate(), m_AttitudeControlSimple.inputSignalRotationRateMeasured());
        
        // PoseEstimation -> AttitudeControlSimple
        m_ControlSystem.connectPorts(m_PoseEstimation->outputSignalAttitude(), m_AttitudeControlSimple.inputSignalAttitudeEstimated());
        
        // PositionControlSimple -> AttitudeControlSimple
        m_ControlSystem.connectPorts(m_PositionControlSimple.outputSignalAttitudeSetpoint(), m_AttitudeControlSimple.inputSignalAttitudeSetpoint());
        
        // PositionControlSimple -> QuadMotorsThrustSimple
        m_ControlSystem.connectPorts(m_PositionControlSimple.outputSignalThrustSetpoint(), m_QuadMotorsThrustSimple.inputSignalThrustSetpoint());
        
        // AttitudeControlSimple -> QuadMotorsThrustSimple
        m_ControlSystem.connectPorts(m_AttitudeControlSimple.outputSignalRotationRateSetpoint(), m_QuadMotorsThrustSimple.inputSignalRotationRateSetpoint());
    }
    
    
    void PoseEstimationView::setupGraphs()
    {
        //velocity rate
        m_VelocityRate->setPlottingHint(QCP::phForceRepaint, false);
        m_VelocityRate->addGraph();
        m_VelocityRate->graph(0)->setPen(QPen(Qt::red));
        m_VelocityRate->addGraph();
        m_VelocityRate->graph(1)->setPen(QPen(Qt::green));
        m_VelocityRate->addGraph();
        m_VelocityRate->graph(2)->setPen(QPen(Qt::blue));
        m_VelocityRate->yAxis->setLabel("accelerometer");

        //rotation rate
        m_RotationRate->setPlottingHint(QCP::phForceRepaint, false);
        m_RotationRate->addGraph();
        m_RotationRate->graph(0)->setPen(QPen(Qt::red));
        m_RotationRate->addGraph();
        m_RotationRate->graph(1)->setPen(QPen(Qt::green));
        m_RotationRate->addGraph();
        m_RotationRate->graph(2)->setPen(QPen(Qt::blue));
        m_RotationRate->yAxis->setLabel("gyroscope");
        
        //magnetic field
        m_MagneticField->setPlottingHint(QCP::phForceRepaint, false);
        m_MagneticField->addGraph();
        m_MagneticField->graph(0)->setPen(QPen(Qt::red));
        m_MagneticField->addGraph();
        m_MagneticField->graph(1)->setPen(QPen(Qt::green));
        m_MagneticField->addGraph();
        m_MagneticField->graph(2)->setPen(QPen(Qt::blue));
        m_MagneticField->yAxis->setLabel("magnetometer");
        
        //barometrics
        //pressure
        m_Barometrics->setPlottingHint(QCP::phForceRepaint, false);
        m_Barometrics->addGraph();
        m_Barometrics->graph(0)->setPen(QPen(Qt::blue));
        m_Barometrics->yAxis->setLabel("barometer");
        
        //attitude est
        m_AttitudeEst->setPlottingHint(QCP::phForceRepaint, false);
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(0)->setPen(QPen(Qt::red));
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(1)->setPen(QPen(Qt::green));
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(2)->setPen(QPen(Qt::blue));
        m_AttitudeEst->yAxis->setLabel("attitude");
        
        //velocity est
        m_VelocityEst->setPlottingHint(QCP::phForceRepaint, false);
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(0)->setPen(QPen(Qt::red));
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(1)->setPen(QPen(Qt::green));
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(2)->setPen(QPen(Qt::blue));
        m_VelocityEst->yAxis->setLabel("velocity");
        
        //position est
        m_PositionEst->setPlottingHint(QCP::phForceRepaint, false);
        m_PositionEst->addGraph();
        m_PositionEst->graph(0)->setPen(QPen(Qt::red));
        m_PositionEst->addGraph();
        m_PositionEst->graph(1)->setPen(QPen(Qt::green));
        m_PositionEst->addGraph();
        m_PositionEst->graph(2)->setPen(QPen(Qt::blue));
        m_PositionEst->yAxis->setLabel("position");
        
        //earth magnetic
        m_EarthMagneticFieldEst->setPlottingHint(QCP::phForceRepaint, false);
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(0)->setPen(QPen(Qt::red));
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(1)->setPen(QPen(Qt::green));
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(2)->setPen(QPen(Qt::blue));
        m_EarthMagneticFieldEst->yAxis->setLabel("magnetic field");
        
    }
    
    void PoseEstimationView::replotGraphs()
    {
        lock_guard<fast_mutex> graphsGuard(m_GraphsGuard);
        
        m_VelocityRate->replot();
        m_RotationRate->replot();
        m_MagneticField->replot();
        m_Barometrics->replot();
        m_AttitudeEst->replot();
        m_VelocityEst->replot();
        m_PositionEst->replot();
        m_EarthMagneticFieldEst->replot();
    }
    
    
    void PoseEstimationView::handleUQuadSensorsDataReceived(intrusive_ptr<common::msg::UQuadSensorsData> sd)
    {
        lock_guard<fast_mutex> dataGuard(m_UQuadSensorsDataGuard);
        m_UQuadSensorsData.push_back(sd);
        if(!m_CurrentUQuadSensorsData)
        {
            m_CurrentUQuadSensorsDataPlayoutTimer.expires_at(asio::steady_timer::clock_type::now());
            startCurrentUQuadSensorsDataPlayoutTimer();
        }
        
    }
    
    void PoseEstimationView::startCurrentUQuadSensorsDataPlayoutTimer()
    {
        if(!Application::instance()->getRemoteControl().isStarted())
            return;
            
        m_CurrentUQuadSensorsData = m_UQuadSensorsData.front();
        m_UQuadSensorsData.pop_front();
        
        base::TimeDuration samplesDuration = chrono::seconds(0);
        for(uint32_t s = 1; s < m_CurrentUQuadSensorsData->uquadSensorsData.size(); ++s)
        {
            base::TimeDuration const tusec = m_CurrentUQuadSensorsData->uquadSensorsData[s].time - m_CurrentUQuadSensorsData->uquadSensorsData[s - 1].time;
            samplesDuration += tusec;
            int const usecs = chrono::duration_cast<chrono::microseconds>(tusec).count();
            if(usecs > 100000)
            {
                int c = 0;
            }
        }
        
        m_CurrentUQuadSensorsDataSampleDuration = samplesDuration / (m_CurrentUQuadSensorsData->uquadSensorsData.size() - 1);
        m_CurrentUQuadSensorsDataSample = 0;
        
        m_CurrentUQuadSensorsDataPlayoutTimer.async_wait(
            bind(&PoseEstimationView::handleCurrentUQuadSensorsDataPlayoutTimer, this, asio::placeholders::error)
        );
        
        std::cout << "startCurrentUQuadSensorsDataPlayoutTimer: " << chrono::duration_cast<chrono::microseconds>(m_CurrentUQuadSensorsDataSampleDuration).count() << std::endl;
    }
    
    void PoseEstimationView::handleCurrentUQuadSensorsDataPlayoutTimer(system::error_code ec)
    {
        if(ec)
            return;
        
        lock_guard<fast_mutex> dataGuard(m_UQuadSensorsDataGuard);
        
        handleUQuadSensorsData(m_CurrentUQuadSensorsData->uquadSensorsData[m_CurrentUQuadSensorsDataSample]);
        
        m_CurrentUQuadSensorsDataPlayoutTimer.expires_at(
            m_CurrentUQuadSensorsDataPlayoutTimer.expires_at() +
            std::chrono::nanoseconds(
                chrono::duration_cast<chrono::nanoseconds>(
                    m_CurrentUQuadSensorsDataSampleDuration
                ).count()
            )
        );
        
        m_CurrentUQuadSensorsDataSample++;
        if(m_CurrentUQuadSensorsDataSample < m_CurrentUQuadSensorsData->uquadSensorsData.size())
        {
            m_CurrentUQuadSensorsDataPlayoutTimer.async_wait(
                bind(&PoseEstimationView::handleCurrentUQuadSensorsDataPlayoutTimer, this, asio::placeholders::error)
            );
        }
        else
        {
            m_CurrentUQuadSensorsData.reset();
            if(m_UQuadSensorsData.empty())
                return;
            startCurrentUQuadSensorsDataPlayoutTimer();
        }
    }
    
    void PoseEstimationView::handleUQuadSensorsData(uquad::hal::UQuadSensorsData const &d)
    {
        float dT = 0.0f;
        double graphTime = 0.0;
        
        {
            lock_guard<fast_mutex> graphsGuard(m_GraphsGuard);
            if(m_SensorsFirstTS == base::TimePoint::min())
            {
                m_SensorsFirstTS = m_SensorsLastTS = d.time;
            }
            
            graphTime = chrono::duration_cast<chrono::microseconds>(d.time - m_SensorsFirstTS).count()/1000000.0;
            dT = chrono::duration_cast<chrono::microseconds>(d.time - m_SensorsLastTS).count()/1000000.0;
            m_SensorsLastTS = d.time;
            
            m_VelocityRate->graph(0)->addData(graphTime, d.velocityRate(0));
            m_VelocityRate->graph(0)->removeDataBefore(graphTime - 3.0);
            m_VelocityRate->graph(1)->addData(graphTime, d.velocityRate(1));
            m_VelocityRate->graph(1)->removeDataBefore(graphTime - 3.0);
            m_VelocityRate->graph(2)->addData(graphTime, d.velocityRate(2));
            m_VelocityRate->graph(2)->removeDataBefore(graphTime - 3.0);
            m_VelocityRate->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
            m_VelocityRate->yAxis->rescale();
            
            m_RotationRate->graph(0)->addData(graphTime, d.rotationRate(0));
            m_RotationRate->graph(0)->removeDataBefore(graphTime - 3.0);
            m_RotationRate->graph(1)->addData(graphTime, d.rotationRate(1));
            m_RotationRate->graph(1)->removeDataBefore(graphTime - 3.0);
            m_RotationRate->graph(2)->addData(graphTime, d.rotationRate(2));
            m_RotationRate->graph(2)->removeDataBefore(graphTime - 3.0);
            m_RotationRate->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
            m_RotationRate->yAxis->rescale();
            
            m_MagneticField->graph(0)->addData(graphTime, d.magneticField(0));
            m_MagneticField->graph(0)->removeDataBefore(graphTime - 3.0);
            m_MagneticField->graph(1)->addData(graphTime, d.magneticField(1));
            m_MagneticField->graph(1)->removeDataBefore(graphTime - 3.0);
            m_MagneticField->graph(2)->addData(graphTime, d.magneticField(2));
            m_MagneticField->graph(2)->removeDataBefore(graphTime - 3.0);
            m_MagneticField->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
            m_MagneticField->yAxis->rescale();
        }
        
        m_CalibrationView->updateAccelerometerCalibration(d.velocityRate);
        m_CalibrationView->updateGyroscopeCalibration(d.rotationRate);
        
        if(m_CalibrationView->updateMagnetometerCalibration(d.magneticField, dT))
        {
            m_UQuadSensors.setMagnetometerCalibration(m_CalibrationView->magnetometerCalibration().calibrationTransform());
        }
        
        m_UQuadSensors.setSensorsData(d);
        m_UQuadSensors.requestUpdate();
        
        Vec3f dir = Vec3f::Zero();
        dir(0) = m_ManualControlX->value()/50.0f - 1.0f;
        dir(1) = m_ManualControlY->value()/50.0f - 1.0f;
        dir(2) = m_ManualControlZ->value()/50.0f - 1.0f;
        m_ManualControlSimple.setDirection(dir);
        
        m_ManualControlSimple.setThrust(m_ManualControlThrust->value()/100.0f);
        m_ManualControlSimple.requestUpdate();
        
        m_ControlSystem.update([this, graphTime](system::error_code e)
        {
            /*
            static uint8_t s_MPOS = 0;
            
            if(!s_MPOS--)
            {
                s_MPOS = 10;
                
                intrusive_ptr<common::msg::UQuadMotorsPower> msgMotorsPower(new common::msg::UQuadMotorsPower());
                msgMotorsPower->motorsPower = motorsThrust;
                Application::instance()->getRemoteControl().sendMessage(msgMotorsPower);
            }
            */
            
            
            /*control::PoseEstimation::Position position = m_PoseEstimationPosition->value();
            
            m_PoseView->setMagneticField(
                QVector3D(
                     m_PoseEstimation->bodyMagneticField(0),
                     m_PoseEstimation->bodyMagneticField(1),
                     m_PoseEstimation->bodyMagneticField(2)
                )
            );*/


            
            control::blocks::PoseEstimation::Attitude attitude = m_PoseEstimationAttitude->value();
            m_PoseView->setUQuadAttitude(QQuaternion(attitude.w(), attitude.x(), attitude.y(), attitude.z()));
            
            control::blocks::PoseEstimation::Attitude attitudeSetpoint = m_PositionControlSimpleAttitudeSetpoint->value();
            m_PoseView->setUQuadAttitudeSetpoint(QQuaternion(attitudeSetpoint.w(), attitudeSetpoint.x(), attitudeSetpoint.y(), attitudeSetpoint.z()));
            
            
            
            control::blocks::QuadMotorsThrust::MotorsPower motorsPower = m_QuadMotorsThrustMotorsPower->value();
            m_PoseView->setMotorsPower(QVector4D(motorsPower(0), motorsPower(1), motorsPower(2), motorsPower(3)));
            
            {
                lock_guard<fast_mutex> graphsGuard(m_GraphsGuard);
            
            
                if(m_PoseEstimationAttitude)
                {
                    Vec3f const eulerAngles = m_PoseEstimationAttitude->value().toRotationMatrix().eulerAngles(0, 1, 2);
                    
                    m_AttitudeEst->graph(0)->addData(graphTime, wrapPi(eulerAngles(0)));
                    m_AttitudeEst->graph(0)->removeDataBefore(graphTime - 3.0);
                    m_AttitudeEst->graph(1)->addData(graphTime, wrapPi(eulerAngles(1)));
                    m_AttitudeEst->graph(1)->removeDataBefore(graphTime - 3.0);
                    m_AttitudeEst->graph(2)->addData(graphTime, wrapPi(eulerAngles(2)));
                    m_AttitudeEst->graph(2)->removeDataBefore(graphTime - 3.0);
                    m_AttitudeEst->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
                    m_AttitudeEst->yAxis->rescale();
                }
                
                if(m_PoseEstimationPosition)
                {
                    Vec3f const position = m_PoseEstimationPosition->value();
                    m_PositionEst->graph(0)->addData(graphTime, position(0));
                    m_PositionEst->graph(0)->removeDataBefore(graphTime - 3.0);
                    m_PositionEst->graph(1)->addData(graphTime, position(1));
                    m_PositionEst->graph(1)->removeDataBefore(graphTime - 3.0);
                    m_PositionEst->graph(2)->addData(graphTime, position(2));
                    m_PositionEst->graph(2)->removeDataBefore(graphTime - 3.0);
                    m_PositionEst->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
                    m_PositionEst->yAxis->rescale();
                }
                
                if(m_PoseEstimationVelocity)
                {
                    Vec3f const velocity = m_PoseEstimationVelocity->value();
                    m_VelocityEst->graph(0)->addData(graphTime, velocity(0));
                    m_VelocityEst->graph(0)->removeDataBefore(graphTime - 3.0);
                    m_VelocityEst->graph(1)->addData(graphTime, velocity(1));
                    m_VelocityEst->graph(1)->removeDataBefore(graphTime - 3.0);
                    m_VelocityEst->graph(2)->addData(graphTime, velocity(2));
                    m_VelocityEst->graph(2)->removeDataBefore(graphTime - 3.0);
                    m_VelocityEst->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
                    m_VelocityEst->yAxis->rescale();
                }
                
                if(m_PoseEstimationEarthMagneticField)
                {
                    Vec3f const earthMagneticField = m_PoseEstimationEarthMagneticField->value();
                    m_EarthMagneticFieldEst->graph(0)->addData(graphTime, earthMagneticField(0));
                    m_EarthMagneticFieldEst->graph(0)->removeDataBefore(graphTime - 3.0);
                    m_EarthMagneticFieldEst->graph(1)->addData(graphTime, earthMagneticField(1));
                    m_EarthMagneticFieldEst->graph(1)->removeDataBefore(graphTime - 3.0);
                    m_EarthMagneticFieldEst->graph(2)->addData(graphTime, earthMagneticField(2));
                    m_EarthMagneticFieldEst->graph(2)->removeDataBefore(graphTime - 3.0);
                    m_EarthMagneticFieldEst->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
                    m_EarthMagneticFieldEst->yAxis->rescale();
                }
                
                if(m_BarometerAltitude)
                {
                    m_Barometrics->graph(0)->addData(graphTime, m_BarometerAltitude->value());
                    m_Barometrics->graph(0)->removeDataBefore(graphTime - 3.0);
                    m_Barometrics->xAxis->setRange(graphTime, 3.0, Qt::AlignRight);
                    m_Barometrics->yAxis->rescale();
                }
            
                
                /*
                m_EarthMagneticFieldEst->graph(0)->addData(value, m_PoseEstimation->bodyMagneticField(0));
                m_EarthMagneticFieldEst->graph(0)->removeDataBefore(value - 3.0);
                m_EarthMagneticFieldEst->graph(1)->addData(value, m_PoseEstimation->bodyMagneticField(1));
                m_EarthMagneticFieldEst->graph(1)->removeDataBefore(value - 3.0);
                m_EarthMagneticFieldEst->graph(2)->addData(value, m_PoseEstimation->bodyMagneticField(2));
                m_EarthMagneticFieldEst->graph(2)->removeDataBefore(value - 3.0);
                m_EarthMagneticFieldEst->xAxis->setRange(value, 3.0, Qt::AlignRight);
                m_EarthMagneticFieldEst->yAxis->rescale();
                */
            }
        });
        
        
        
        
        QMetaObject::invokeMethod(
            this,
            "updatePoseEstimationUI",
            Qt::QueuedConnection
        );
    }
    
} //namespace local
} //namespace ctrlpe