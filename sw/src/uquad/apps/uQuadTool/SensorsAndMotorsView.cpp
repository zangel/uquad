#include "SensorsAndMotorsView.h"
#include "Application.h"

namespace uquad
{
namespace tool
{

    SensorsAndMotorsView::SensorsAndMotorsView(QMainWindow *mw)
        : QWidget(mw)
        , Ui::SensorsAndMotorsView()
    {
        setupUi(this);
        
        setupMotorControls();
        Application::instance()->getUQuadDevice().m_DataReadySignal.connect(
            bind(&SensorsAndMotorsView::uquadDeviceDataReady, this)
        );
        
        Application::instance()->getUQuadDevice().m_StartStopSignal.connect(
            bind(&SensorsAndMotorsView::uquadDeviceStarted, this, _1)
        );
    }
    
    SensorsAndMotorsView::~SensorsAndMotorsView()
    {
    }
    
    void SensorsAndMotorsView::setupGraphs()
    {
        //acceleration
        m_Acceleration->addGraph();
        m_Acceleration->graph(0)->setPen(QPen(Qt::red));
        m_Acceleration->addGraph();
        m_Acceleration->graph(1)->setPen(QPen(Qt::green));
        m_Acceleration->addGraph();
        m_Acceleration->graph(2)->setPen(QPen(Qt::blue));
        
        //rotation rate
        m_RotationRate->addGraph();
        m_RotationRate->graph(0)->setPen(QPen(Qt::red));
        m_RotationRate->addGraph();
        m_RotationRate->graph(1)->setPen(QPen(Qt::green));
        m_RotationRate->addGraph();
        m_RotationRate->graph(2)->setPen(QPen(Qt::blue));
        
        //magnetic field
        m_MagneticField->addGraph();
        m_MagneticField->graph(0)->setPen(QPen(Qt::red));
        m_MagneticField->addGraph();
        m_MagneticField->graph(1)->setPen(QPen(Qt::green));
        m_MagneticField->addGraph();
        m_MagneticField->graph(2)->setPen(QPen(Qt::blue));
        
        //atm pressure
        m_AtmPressure->addGraph();
        m_AtmPressure->graph(0)->setPen(QPen(Qt::blue));
        
        //atm temperature
        m_AtmTemperature->addGraph();
        m_AtmTemperature->graph(0)->setPen(QPen(Qt::blue));
        
    }
    
    void SensorsAndMotorsView::replotGraphs()
    {
        unique_lock<mutex> lock(m_SensorDataMutex);
        
        m_Acceleration->replot();
        m_RotationRate->replot();
        m_MagneticField->replot();
        m_AtmPressure->replot();
        m_AtmTemperature->replot();
        
        
        //update();
    }
    
    void SensorsAndMotorsView::setupMotorControls()
    {
        m_Motor0Control->setEnabled(Application::instance()->getUQuadDevice().isStarted());
        connect(
            m_Motor0Control, SIGNAL(valueChanged(int)),
            this, SLOT(motor0ValueChanged())
        );
        m_Motor1Control->setEnabled(Application::instance()->getUQuadDevice().isStarted());
        connect(
            m_Motor1Control, SIGNAL(valueChanged(int)),
            this, SLOT(motor1ValueChanged())
        );
        m_Motor2Control->setEnabled(Application::instance()->getUQuadDevice().isStarted());
        connect(
            m_Motor2Control, SIGNAL(valueChanged(int)),
            this, SLOT(motor2ValueChanged())
        );
        m_Motor3Control->setEnabled(Application::instance()->getUQuadDevice().isStarted());
        connect(
            m_Motor3Control, SIGNAL(valueChanged(int)),
            this, SLOT(motor3ValueChanged())
        );
    }
    
    void SensorsAndMotorsView::uquadDeviceStarted(bool started)
    {
        if(started)
        {
            m_Acceleration->graph(0)->clearData();
            m_Acceleration->graph(1)->clearData();
            m_Acceleration->graph(2)->clearData();
            
            m_RotationRate->graph(0)->clearData();
            m_RotationRate->graph(1)->clearData();
            m_RotationRate->graph(2)->clearData();
            
            m_MagneticField->graph(0)->clearData();
            m_MagneticField->graph(1)->clearData();
            m_MagneticField->graph(2)->clearData();
            
            m_AtmPressure->graph(0)->clearData();
            
            m_AtmTemperature->graph(0)->clearData();
            
            m_Motor0Control->setValue(0);
            m_Motor1Control->setValue(0);
            m_Motor2Control->setValue(0);
            m_Motor3Control->setValue(0);
            
            m_SensorsFirstTS = TimePoint::min();
            
            m_AccelerationStats.clear();
            m_AccelerationStats.resize(3);
        }
        
        m_Motor0Control->setEnabled(started);
        m_Motor1Control->setEnabled(started);
        m_Motor2Control->setEnabled(started);
        m_Motor3Control->setEnabled(started);
    }
    
    void SensorsAndMotorsView::uquadDeviceDataReady()
    {
        {
            unique_lock<mutex> lock(m_SensorDataMutex);
            TimePoint now = Clock::now();
            
            if(m_SensorsFirstTS == TimePoint::min())
                m_SensorsFirstTS = now;
            
            hal::UQuadDevice &device = Application::instance()->getUQuadDevice();
            
            double value = chrono::duration_cast<chrono::microseconds>(now - m_SensorsFirstTS).count()/1000000.0;
            
            
            m_Acceleration->graph(0)->addData(value, device.m_Acceleration(0));
            m_Acceleration->graph(0)->removeDataBefore(value - 3.0);
            m_Acceleration->graph(1)->addData(value, device.m_Acceleration(1));
            m_Acceleration->graph(1)->removeDataBefore(value - 3.0);
            m_Acceleration->graph(2)->addData(value, device.m_Acceleration(2));
            m_Acceleration->graph(2)->removeDataBefore(value - 3.0);
            m_Acceleration->xAxis->setRange(value, 3.0, Qt::AlignRight);
            m_Acceleration->yAxis->rescale();
            m_AccelerationStats[0](device.m_Acceleration(0));
            m_AccelerationStats[1](device.m_Acceleration(1));
            m_AccelerationStats[2](device.m_Acceleration(2));
            m_AccelerationVarX->setText(QString("%1").arg(accumulators::variance(m_AccelerationStats[0])));
            m_AccelerationVarY->setText(QString("%1").arg(accumulators::variance(m_AccelerationStats[1])));
            m_AccelerationVarZ->setText(QString("%1").arg(accumulators::variance(m_AccelerationStats[2])));
            
            
            
            m_RotationRate->graph(0)->addData(value, device.m_RotationRate(0));
            m_RotationRate->graph(0)->removeDataBefore(value - 3.0);
            m_RotationRate->graph(1)->addData(value, device.m_RotationRate(1));
            m_RotationRate->graph(1)->removeDataBefore(value - 3.0);
            m_RotationRate->graph(2)->addData(value, device.m_RotationRate(2));
            m_RotationRate->graph(2)->removeDataBefore(value - 3.0);
            m_RotationRate->xAxis->setRange(value, 3.0, Qt::AlignRight);
            m_RotationRate->yAxis->rescale();
            
            m_MagneticField->graph(0)->addData(value, device.m_MagneticField(0));
            m_MagneticField->graph(0)->removeDataBefore(value - 3.0);
            m_MagneticField->graph(1)->addData(value, device.m_MagneticField(1));
            m_MagneticField->graph(1)->removeDataBefore(value - 3.0);
            m_MagneticField->graph(2)->addData(value, device.m_MagneticField(2));
            m_MagneticField->graph(2)->removeDataBefore(value - 3.0);
            m_MagneticField->xAxis->setRange(value, 3.0, Qt::AlignRight);
            m_MagneticField->yAxis->rescale();
            
            m_AtmPressure->graph(0)->addData(value, device.m_AtmPressure);
            m_AtmPressure->graph(0)->removeDataBefore(value - 3.0);
            m_AtmPressure->xAxis->setRange(value, 3.0, Qt::AlignRight);
            m_AtmPressure->yAxis->rescale();
            
            m_AtmTemperature->graph(0)->addData(value, device.m_AtmTemperature);
            m_AtmTemperature->graph(0)->removeDataBefore(value - 3.0);
            m_AtmTemperature->xAxis->setRange(value, 3.0, Qt::AlignRight);
            m_AtmTemperature->yAxis->rescale();
        
        }
        
        static uint8_t _invoke_count = 0;
        _invoke_count++;
        
        if((_invoke_count >> 1) & 0x01)
        {
            QMetaObject::invokeMethod(
                this,
                "replotGraphs",
                Qt::QueuedConnection
            );
        }
        
    }
    
    void SensorsAndMotorsView::motor0ValueChanged()
    {
        m_Motor0Value->display(m_Motor0Control->value());
        setMotorsPWM();
    }
    
    void SensorsAndMotorsView::motor1ValueChanged()
    {
        m_Motor1Value->display(m_Motor1Control->value());
        setMotorsPWM();
    }
    
    void SensorsAndMotorsView::motor2ValueChanged()
    {
        m_Motor2Value->display(m_Motor2Control->value());
        setMotorsPWM();
    }
    
    void SensorsAndMotorsView::motor3ValueChanged()
    {
        m_Motor3Value->display(m_Motor3Control->value());
        setMotorsPWM();
    }
    
    void SensorsAndMotorsView::setMotorsPWM()
    {
        Application::instance()->getUQuadDevice().setMotorsPWM(
            m_Motor0Control->value()/100.0f,
            m_Motor1Control->value()/100.0f,
            m_Motor2Control->value()/100.0f,
            m_Motor3Control->value()/100.0f
        );
    }

} //namespace tool
} //namespace uquad