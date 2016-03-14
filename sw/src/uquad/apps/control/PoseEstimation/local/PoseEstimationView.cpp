#include "PoseEstimationView.h"
#include "Application.h"
#include "PoseView.h"
#include "AccelerometerCalibrationView.h"
#include <uquad/control/SystemLibrary.h>
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
        , m_PoseEstimation(SI(control::SystemLibrary).createPoseEstimation("ikf"))
        , m_UQuadSensorsDataGuard()
        , m_UQuadSensorsData()
        , m_CurrentUQuadSensorsData()
        , m_CurrentUQuadSensorsDataPlayoutTimer(Application::instance()->getRunloop()->ios())
        , m_CurrentUQuadSensorsDataSampleDuration()
        , m_CurrentUQuadSensorsDataSample(0)
    {
        setupUi(this);
        setupGraphs();
        Application::instance()->getRemoteControl().remoteControlDelegate() += this;
        
        connect(
            m_SaveCalibration, SIGNAL(clicked()),
            this, SLOT(onSaveCalibrationClicked())
        );
        
        connect(
            m_ResetCalibration, SIGNAL(clicked()),
            this, SLOT(onResetCalibrationClicked())
        );
        updatePoseEstimationUI();
        
    }
    
    PoseEstimationView::~PoseEstimationView()
    {
        Application::instance()->getRemoteControl().remoteControlDelegate() -= this;
    }
    
    void PoseEstimationView::onRemoteControlStarted()
    {
        m_PoseEstimation->prepare();
        
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
        m_PoseEstimation->unprepare();
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
        m_ResetCalibration->setEnabled(m_AccelerometerCalibrationView->isCalibrated());
        m_SaveCalibration->setEnabled(m_AccelerometerCalibrationView->isCalibrated());
    }
    
    void PoseEstimationView::onResetCalibrationClicked()
    {
        m_AccelerometerCalibrationView->reset();
        updatePoseEstimationUI();
    }
    
    
    void PoseEstimationView::onSaveCalibrationClicked()
    {
        QString calibFileName = QFileDialog::getSaveFileName(this, tr("Save calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ofstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::OArchive ccArchive(ccFile);
            ccArchive & m_AccelerometerCalibrationView->calibration();
        }
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
        
        //rotation rate
        m_RotationRate->setPlottingHint(QCP::phForceRepaint, false);
        m_RotationRate->addGraph();
        m_RotationRate->graph(0)->setPen(QPen(Qt::red));
        m_RotationRate->addGraph();
        m_RotationRate->graph(1)->setPen(QPen(Qt::green));
        m_RotationRate->addGraph();
        m_RotationRate->graph(2)->setPen(QPen(Qt::blue));
        
        //magnetic field
        m_MagneticField->setPlottingHint(QCP::phForceRepaint, false);
        m_MagneticField->addGraph();
        m_MagneticField->graph(0)->setPen(QPen(Qt::red));
        m_MagneticField->addGraph();
        m_MagneticField->graph(1)->setPen(QPen(Qt::green));
        m_MagneticField->addGraph();
        m_MagneticField->graph(2)->setPen(QPen(Qt::blue));
        
        //barometrics
        //pressure
        m_Barometrics->setPlottingHint(QCP::phForceRepaint, false);
        m_Barometrics->addGraph();
        m_Barometrics->graph(0)->setPen(QPen(Qt::blue));
        
        //attitude est
        m_AttitudeEst->setPlottingHint(QCP::phForceRepaint, false);
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(0)->setPen(QPen(Qt::red));
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(1)->setPen(QPen(Qt::green));
        m_AttitudeEst->addGraph();
        m_AttitudeEst->graph(2)->setPen(QPen(Qt::blue));
        
        //velocity est
        m_VelocityEst->setPlottingHint(QCP::phForceRepaint, false);
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(0)->setPen(QPen(Qt::red));
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(1)->setPen(QPen(Qt::green));
        m_VelocityEst->addGraph();
        m_VelocityEst->graph(2)->setPen(QPen(Qt::blue));
        
        //position est
        m_PositionEst->setPlottingHint(QCP::phForceRepaint, false);
        m_PositionEst->addGraph();
        m_PositionEst->graph(0)->setPen(QPen(Qt::red));
        m_PositionEst->addGraph();
        m_PositionEst->graph(1)->setPen(QPen(Qt::green));
        m_PositionEst->addGraph();
        m_PositionEst->graph(2)->setPen(QPen(Qt::blue));
        
        //earth magnetic
        m_EarthMagneticFieldEst->setPlottingHint(QCP::phForceRepaint, false);
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(0)->setPen(QPen(Qt::red));
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(1)->setPen(QPen(Qt::green));
        m_EarthMagneticFieldEst->addGraph();
        m_EarthMagneticFieldEst->graph(2)->setPen(QPen(Qt::blue));
        
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
        lock_guard<fast_mutex> graphsGuard(m_GraphsGuard);
        if(m_SensorsFirstTS == base::TimePoint::min())
            m_SensorsFirstTS = d.time;
        
        m_AccelerometerCalibrationView->setRawPoint(d.velocityRate);
        
        uquad::hal::UQuadSensorsData cd = d;
        cd.velocityRate = m_AccelerometerCalibrationView->calibratedPoint();
        
        m_PoseEstimation->processUQuadSensorsData(cd);
        
        m_PoseView->setUQuadAttitude(
            QQuaternion(
                m_PoseEstimation->attitude.w(),
                m_PoseEstimation->attitude.x(),
                m_PoseEstimation->attitude.y(),
                m_PoseEstimation->attitude.z()
            )
        );
        
        m_PoseView->setMagneticField(
            QVector3D(
                 m_PoseEstimation->bodyMagneticField(0),
                 m_PoseEstimation->bodyMagneticField(1),
                 m_PoseEstimation->bodyMagneticField(2)
            )
        );

        double value = chrono::duration_cast<chrono::microseconds>(cd.time - m_SensorsFirstTS).count()/1000000.0;
        
        m_VelocityRate->graph(0)->addData(value, cd.velocityRate(0));
        m_VelocityRate->graph(0)->removeDataBefore(value - 3.0);
        m_VelocityRate->graph(1)->addData(value, cd.velocityRate(1));
        m_VelocityRate->graph(1)->removeDataBefore(value - 3.0);
        m_VelocityRate->graph(2)->addData(value, cd.velocityRate(2));
        m_VelocityRate->graph(2)->removeDataBefore(value - 3.0);
        m_VelocityRate->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_VelocityRate->yAxis->rescale();
        
        m_RotationRate->graph(0)->addData(value, cd.rotationRate(0));
        m_RotationRate->graph(0)->removeDataBefore(value - 3.0);
        m_RotationRate->graph(1)->addData(value, cd.rotationRate(1));
        m_RotationRate->graph(1)->removeDataBefore(value - 3.0);
        m_RotationRate->graph(2)->addData(value, cd.rotationRate(2));
        m_RotationRate->graph(2)->removeDataBefore(value - 3.0);
        m_RotationRate->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_RotationRate->yAxis->rescale();
        
        m_MagneticField->graph(0)->addData(value, cd.magneticField(0));
        m_MagneticField->graph(0)->removeDataBefore(value - 3.0);
        m_MagneticField->graph(1)->addData(value, cd.magneticField(1));
        m_MagneticField->graph(1)->removeDataBefore(value - 3.0);
        m_MagneticField->graph(2)->addData(value, cd.magneticField(2));
        m_MagneticField->graph(2)->removeDataBefore(value - 3.0);
        m_MagneticField->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_MagneticField->yAxis->rescale();
        
        m_Barometrics->graph(0)->addData(value, cd.baroPressure);
        m_Barometrics->graph(0)->removeDataBefore(value - 3.0);
        m_Barometrics->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_Barometrics->yAxis->rescale();
        
        
        Vec3f const eulerAngles = m_PoseEstimation->attitude.toRotationMatrix().eulerAngles(0, 1, 2);
        
        m_AttitudeEst->graph(0)->addData(value, wrapPi(eulerAngles(0)));
        m_AttitudeEst->graph(0)->removeDataBefore(value - 3.0);
        m_AttitudeEst->graph(1)->addData(value, wrapPi(eulerAngles(1)));
        m_AttitudeEst->graph(1)->removeDataBefore(value - 3.0);
        m_AttitudeEst->graph(2)->addData(value, wrapPi(eulerAngles(2)));
        m_AttitudeEst->graph(2)->removeDataBefore(value - 3.0);
        m_AttitudeEst->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_AttitudeEst->yAxis->rescale();
        
        m_VelocityEst->graph(0)->addData(value, m_PoseEstimation->velocity(0));
        m_VelocityEst->graph(0)->removeDataBefore(value - 3.0);
        m_VelocityEst->graph(1)->addData(value, m_PoseEstimation->velocity(1));
        m_VelocityEst->graph(1)->removeDataBefore(value - 3.0);
        m_VelocityEst->graph(2)->addData(value, m_PoseEstimation->velocity(2));
        m_VelocityEst->graph(2)->removeDataBefore(value - 3.0);
        m_VelocityEst->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_VelocityEst->yAxis->rescale();
        
        m_PositionEst->graph(0)->addData(value, m_PoseEstimation->position(0));
        m_PositionEst->graph(0)->removeDataBefore(value - 3.0);
        m_PositionEst->graph(1)->addData(value, m_PoseEstimation->position(1));
        m_PositionEst->graph(1)->removeDataBefore(value - 3.0);
        m_PositionEst->graph(2)->addData(value, m_PoseEstimation->position(2));
        m_PositionEst->graph(2)->removeDataBefore(value - 3.0);
        m_PositionEst->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_PositionEst->yAxis->rescale();
        
        
        m_EarthMagneticFieldEst->graph(0)->addData(value, m_PoseEstimation->bodyMagneticField(0));
        m_EarthMagneticFieldEst->graph(0)->removeDataBefore(value - 3.0);
        m_EarthMagneticFieldEst->graph(1)->addData(value, m_PoseEstimation->bodyMagneticField(1));
        m_EarthMagneticFieldEst->graph(1)->removeDataBefore(value - 3.0);
        m_EarthMagneticFieldEst->graph(2)->addData(value, m_PoseEstimation->bodyMagneticField(2));
        m_EarthMagneticFieldEst->graph(2)->removeDataBefore(value - 3.0);
        m_EarthMagneticFieldEst->xAxis->setRange(value, 3.0, Qt::AlignRight);
        m_EarthMagneticFieldEst->yAxis->rescale();
        
        QMetaObject::invokeMethod(
            this,
            "updatePoseEstimationUI",
            Qt::QueuedConnection
        );

    }
    
} //namespace local
} //namespace ctrlpe