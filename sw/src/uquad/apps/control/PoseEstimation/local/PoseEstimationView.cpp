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
        , m_BarometerRelativeAttitude()
        , m_PoseEstimation(SI(control::BlockLibrary).createPoseEstimation("ekf2"))
        , m_PoseEstimationAttitude()
        , m_PoseEstimationPosition()
        , m_PoseEstimationEarthMagneticField()
        , m_AttitudeControlSimple()
        , m_PositionControlSimple()
        , m_QuadMotorsThrustSimple()
        , m_QuadMotorsThrustMotors()
        , m_UQuadSensorsDataGuard()
        , m_UQuadSensorsData()
        , m_CurrentUQuadSensorsData()
        , m_CurrentUQuadSensorsDataPlayoutTimer(Application::instance()->getRunloop()->ios())
        , m_CurrentUQuadSensorsDataSampleDuration()
        , m_CurrentUQuadSensorsDataSample(0)
    {
        
        intrusive_ptr_add_ref(&m_ControlSystem);
        intrusive_ptr_add_ref(&m_UQuadSensors);
        
        m_BarometerRelativeAttitude = m_UQuadSensors.typedOutputSignal<control::Barometer::Altitude>(control::Barometer::relativeAltitudeName());
        
        m_PoseEstimationAttitude = m_PoseEstimation->typedOutputSignal<control::PoseEstimation::Attitude>(control::PoseEstimation::attitudeName());
        m_PoseEstimationPosition = m_PoseEstimation->typedOutputSignal<control::PoseEstimation::Position>(control::PoseEstimation::positionName());
        m_PoseEstimationVelocity = m_PoseEstimation->typedOutputSignal<control::PoseEstimation::Velocity>(control::PoseEstimation::velocityName());
        m_PoseEstimationEarthMagneticField = m_PoseEstimation->typedOutputSignal<control::PoseEstimation::MagneticField>(control::PoseEstimation::earthMagneticFieldName());
        
        intrusive_ptr_add_ref(&m_AttitudeControlSimple);
        intrusive_ptr_add_ref(&m_PositionControlSimple);
        intrusive_ptr_add_ref(&m_QuadMotorsThrustSimple);
        
        m_QuadMotorsThrustMotors = m_QuadMotorsThrustSimple.typedOutputSignal<control::QuadMotorsThrust::Motors>(control::QuadMotorsThrust::motorsName());
        
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
        
        m_SensorsFirstTS = base::TimePoint::min();
        
    }
    
    void PoseEstimationView::onRemoteControlStopped()
    {
        m_CurrentUQuadSensorsDataPlayoutTimer.cancel();
        m_CurrentUQuadSensorsData.reset();
        m_UQuadSensorsData.clear();
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
        m_ControlSystem.addBlock(&m_AttitudeControlSimple);     //2
        m_ControlSystem.addBlock(&m_PositionControlSimple);     //3
        m_ControlSystem.addBlock(&m_QuadMotorsThrustSimple);    //4
        
        m_ControlSystem.connectBlocks(&m_UQuadSensors, m_PoseEstimation);
        
        m_ControlSystem.connectBlocks(&m_UQuadSensors, &m_PositionControlSimple);
        m_ControlSystem.connectBlocks(m_PoseEstimation, &m_PositionControlSimple);
        
        m_ControlSystem.connectBlocks(&m_UQuadSensors, &m_AttitudeControlSimple);
        m_ControlSystem.connectBlocks(m_PoseEstimation, &m_AttitudeControlSimple);
        //m_ControlSystem.connectBlocks(&m_PositionControlSimple, &m_AttitudeControlSimple);
        
        
        m_ControlSystem.connectBlocks(&m_PositionControlSimple, &m_QuadMotorsThrustSimple);
        m_ControlSystem.connectBlocks(&m_AttitudeControlSimple, &m_QuadMotorsThrustSimple);
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
            samplesDuration += m_CurrentUQuadSensorsData->uquadSensorsData[s].time - m_CurrentUQuadSensorsData->uquadSensorsData[s - 1].time;
        }
        
        m_CurrentUQuadSensorsDataSampleDuration = samplesDuration / (m_CurrentUQuadSensorsData->uquadSensorsData.size() - 1);
        m_CurrentUQuadSensorsDataSample = 0;
        
        m_CurrentUQuadSensorsDataPlayoutTimer.async_wait(
            bind(&PoseEstimationView::handleCurrentUQuadSensorsDataPlayoutTimer, this, asio::placeholders::error)
        );
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


            
            control::PoseEstimation::Attitude attitude = m_PoseEstimationAttitude->value();
            m_PoseView->setUQuadAttitude(QQuaternion(attitude.w(), attitude.x(), attitude.y(), attitude.z()));
            
            control::QuadMotorsThrust::Motors motorsThrust = m_QuadMotorsThrustMotors->value();
            m_PoseView->setMotorsThrust(QVector4D(motorsThrust(0), motorsThrust(1), motorsThrust(2), motorsThrust(3)));
            
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
                
                if(m_BarometerRelativeAttitude)
                {
                    m_Barometrics->graph(0)->addData(graphTime, m_BarometerRelativeAttitude->value());
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