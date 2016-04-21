#ifndef CTRLPE_LOCAL_CALIBRATION_VIEW_H
#define CTRLPE_LOCAL_CALIBRATION_VIEW_H

#include "View3D.h"

namespace ctrlpe
{
namespace local
{
    class CalibrationView
        : public View3D
    {
    private:
        Q_OBJECT
    protected:
    
    public:
        CalibrationView(QWidget *parent = 0);
        ~CalibrationView();
        
        bool isAccelerometerCalibrated() const;
        bool isGyroscopeCalibrated() const;
        bool isMagnetometerCalibrated() const;
        
        inline control::AccelerometerCalibration const& accelerometerCalibration() const { return m_AccelerometerCalibration; }
        inline control::GyroscopeCalibration const& gyroscopeCalibration() const { return m_GyroscopeCalibration; }
        inline control::MagnetometerCalibration const& magnetometerCalibration() const { return m_MagnetometerCalibration; }
        
        void resetAccelerometerCalibration();
        void resetGyroscopeCalibration();
        void resetMagnetometerCalibration();
        
        bool updateAccelerometerCalibration(Vec3f vr);
        Vec3f calibratedVelocityRate() const;
        
        bool updateGyroscopeCalibration(Vec3f rr);
        Vec3f calibratedRotationRate() const;
        
        bool updateMagnetometerCalibration(Vec3f mf, float dt);
        Vec3f calibratedMagneticField() const;
    
    protected:
        void drawView();
        void drawEllipsoidPoints();
        
    private:
        Vec3f m_VelocityRate;
        Vec3f m_RotationRate;
        Vec3f m_MagneticField;
        
        control::AccelerometerCalibration m_AccelerometerCalibration;
        control::GyroscopeCalibration m_GyroscopeCalibration;
        control::MagnetometerCalibration m_MagnetometerCalibration;
        
    };
    
} //namespace local
} //namespace ctrlpe
        
#endif //CTRLPE_LOCAL_CALIBRATION_VIEW_H
