#ifndef UQUAD_TOOL_SENSORS_AND_MOTORS_VIEW_H
#define UQUAD_TOOL_SENSORS_AND_MOTORS_VIEW_H

#include "Config.h"
#include "ui_SensorsAndMotorsView.h"

namespace uquad
{
namespace tool
{
    class SensorsAndMotorsView
        : public QWidget
        , protected Ui::SensorsAndMotorsView
    {
    private:
        typedef chrono::high_resolution_clock Clock;
        typedef chrono::time_point<Clock> TimePoint;
        
        Q_OBJECT
    protected:
    
    public:
        SensorsAndMotorsView(QMainWindow *mw);
        ~SensorsAndMotorsView();
        
    protected:
        void setupGraphs();
        Q_INVOKABLE void replotGraphs();
        void setupMotorControls();
        
        void uquadDeviceStarted(bool started);
        void uquadDeviceDataReady();
        
        void setMotorsPWM();
        
        
        
        
    protected slots:
        void motor0ValueChanged();
        void motor1ValueChanged();
        void motor2ValueChanged();
        void motor3ValueChanged();
        
        
        
    private:
        mutex m_SensorDataMutex;
        TimePoint m_SensorsFirstTS;
        std::vector<stats_t> m_AccelerationStats;
    };
    
    
} //namespace tool
} //namespace uquad

#endif //UQUAD_TOOL_SENSORS_AND_MOTORS_VIEW_H
  