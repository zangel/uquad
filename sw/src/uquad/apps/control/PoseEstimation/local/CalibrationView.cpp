#include "CalibrationView.h"

namespace ctrlpe
{
namespace local
{
    CalibrationView::CalibrationView(QWidget *parent)
        : View3D(parent)
        , m_AccelerometerCalibration()
        , m_GyroscopeCalibration()
        , m_MagnetometerCalibration()
    {
        m_AccelerometerCalibration.setStatsWindowSize(10);
        //m_AccelerometerCalibration.setStatsSD(3.0e-3f);
        m_AccelerometerCalibration.reset();
        
        m_GyroscopeCalibration.setStatsWindowSize(10);
        //m_GyroscopeCalibration.setStatsSD(3.0e-3f);
        m_GyroscopeCalibration.reset();
        
        m_MagnetometerCalibration.reset();
    }
    
    CalibrationView::~CalibrationView()
    {
    }
    
    bool CalibrationView::isAccelerometerCalibrated() const
    {
        return m_AccelerometerCalibration.samplesQuality() >= 1.0f;
    }
    
    bool CalibrationView::isGyroscopeCalibrated() const
    {
        return m_GyroscopeCalibration.samplesQuality() >= 0.5f;
    }
    
    bool CalibrationView::isMagnetometerCalibrated() const
    {
        return m_MagnetometerCalibration.calibrationQuality() >= 0.5f;
    }
    
    void CalibrationView::resetAccelerometerCalibration()
    {
        m_AccelerometerCalibration.reset();
    }
    
    void CalibrationView::resetGyroscopeCalibration()
    {
        m_GyroscopeCalibration.reset();
    }
    
    void CalibrationView::resetMagnetometerCalibration()
    {
        m_MagnetometerCalibration.reset();
    }
    
    bool CalibrationView::updateAccelerometerCalibration(Vec3f vr)
    {
        m_VelocityRate = vr;
        m_AccelerometerCalibration.processSample(m_VelocityRate);
        
        if(isAccelerometerCalibrated())
        {
            m_AccelerometerCalibration.update();
            requestDraw();
            return true;
        }
        requestDraw();
        return false;
    }
    
    Vec3f CalibrationView::calibratedVelocityRate() const
    {
        return m_AccelerometerCalibration.calibrationTransform() * m_VelocityRate;
    }
    
    bool CalibrationView::updateGyroscopeCalibration(Vec3f rr)
    {
        m_RotationRate = rr;
        m_GyroscopeCalibration.processSample(m_RotationRate);
        
        if(isGyroscopeCalibrated())
        {
            requestDraw();
            m_GyroscopeCalibration.update();
            return true;
        }
        requestDraw();
        return false;
    }
    
    Vec3f CalibrationView::calibratedRotationRate() const
    {
        return m_GyroscopeCalibration.calibrationTransform() * m_RotationRate;
    }
    
    bool CalibrationView::updateMagnetometerCalibration(Vec3f mf, float dt)
    {
        m_MagneticField = mf;
        if(m_MagnetometerCalibration.update(m_MagneticField, dt))
        {
            requestDraw();
            return true;
        }
        requestDraw();
        return false;
    }
    
    Vec3f CalibrationView::calibratedMagneticField() const
    {
        return m_MagnetometerCalibration.calibrationTransform() * m_MagneticField;
    }
    
    
    void CalibrationView::drawView()
    {
        drawXYGrid(QVector2D(2, 2));
        drawAxes();
        
        Vec3f meanPoint = m_AccelerometerCalibration.meanSample();
        
        glPointSize(5.0f);
        glColor3f(1, 1, 0);
        glBegin(GL_POINTS);
        glVertex3f(
            meanPoint.x(),
            meanPoint.y(),
            meanPoint.z()
        );
        glEnd();
        
        
        if(isAccelerometerCalibrated())
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glMultMatrixf(m_AccelerometerCalibration.mapTransform().matrix().data());
            
            
            drawAxes();
            
            glColor3f(0.95f, 0.92f, 0.0f);
            glPointSize(0.5f);
            drawSphere();
            glPopMatrix();

            Vec3f calibrated = m_AccelerometerCalibration.calibrationTransform() * meanPoint;
            
            glPointSize(5.0f);
            glColor3f(0, 1, 1);
            glBegin(GL_POINTS);
            glVertex3f(
                calibrated.x(),
                calibrated.y(),
                calibrated.z()
            );
            glEnd();
        }
        else
        {
            drawEllipsoidPoints();
        }
    }
    
    void CalibrationView::drawEllipsoidPoints()
    {
        glPointSize(5.0f);
        for(std::size_t p = 0; p < control::AccelerometerCalibration::DIRECTION_COUNT; ++p)
        {
            if(m_AccelerometerCalibration.isDirectionSampled(p))
            {
                glColor3f(0, 1, 0);
            }
            else
            {
                glColor3f(0.75f, 0.75f, 0.75f);
            }
            
            Vec3f pointDirection(control::AccelerometerCalibration::direction(p));
            glBegin(GL_POINTS);
            glVertex3f(
                pointDirection.x(),
                pointDirection.y(),
                pointDirection.z()
            );
            glEnd();
        }
    }
  

} //namespace local
} //namespace ctrlpe
