#ifndef CTRLPE_LOCAL_ACCELEROMETERCALIBRATIONVIEW_H
#define CTRLPE_LOCAL_ACCELEROMETERCALIBRATIONVIEW_H

#include "View3D.h"

namespace ctrlpe
{
namespace local
{
    class AccelerometerCalibrationView
        : public View3D
    {
    private:
        Q_OBJECT
    protected:
    
    public:
        AccelerometerCalibrationView(QWidget *parent = 0);
        ~AccelerometerCalibrationView();
        
        bool isCalibrated() const;
        
        inline control::AccelerometerCalibration const& calibration() const { return m_Calibration; }
        
        void reset();
        void setRawPoint(Vec3f point);
        Vec3f calibratedPoint() const;
        
    protected:
        void drawView();
        void drawEllipsoidPoints();
        
    private:
        Vec3f m_RawPoint;
        control::AccelerometerCalibration m_Calibration;
    };
    
} //namespace local
} //namespace ctrlpe
        
#endif //CTRLPE_LOCAL_ACCELEROMETERCALIBRATIONVIEW_H
