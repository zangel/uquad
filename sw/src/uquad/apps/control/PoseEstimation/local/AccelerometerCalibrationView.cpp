#include "AccelerometerCalibrationView.h"

namespace ctrlpe
{
namespace local
{
    AccelerometerCalibrationView::AccelerometerCalibrationView(QWidget *parent)
        : View3D(parent)
        , m_Calibration()
    {
        m_Calibration.setStatsWindowSize(10);
        //m_AccelerometerCalibration.setStatsSD(3.0e-3f);
        m_Calibration.reset();
    }
    
    AccelerometerCalibrationView::~AccelerometerCalibrationView()
    {
    }
    
    bool AccelerometerCalibrationView::isCalibrated() const
    {
        return m_Calibration.pointsQuality() >= 0.5f;
    }
    
    void AccelerometerCalibrationView::reset()
    {
        m_Calibration.reset();
    }
    
    void AccelerometerCalibrationView::setRawPoint(Vec3f point)
    {
        m_RawPoint = point;
        //point*=G_const;
        m_Calibration.process(m_RawPoint);
        if(isCalibrated())
        {
            m_Calibration.update();
        }
        requestDraw();
    }
    
    Vec3f AccelerometerCalibrationView::calibratedPoint() const
    {
        return m_Calibration.calibrationTransform() * m_RawPoint;
    }
    
    void AccelerometerCalibrationView::drawView()
    {
        drawXYGrid(QVector2D(2, 2));
        drawAxes();
        
        Vec3f meanPoint = m_Calibration.meanPoint();
        
        glPointSize(5.0f);
        glColor3f(1, 1, 0);
        glBegin(GL_POINTS);
        glVertex3f(
            meanPoint.x(),
            meanPoint.y(),
            meanPoint.z()
        );
        glEnd();
        
        
        if(isCalibrated())
        {
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glMultMatrixf(m_Calibration.mapTransform().matrix().data());
            
            
            drawAxes();
            
            glColor3f(0.95f, 0.92f, 0.0f);
            glPointSize(0.5f);
            drawSphere();
            glPopMatrix();

            Vec3f calibrated = m_Calibration.calibrationTransform() * meanPoint;
            
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
    
    void AccelerometerCalibrationView::drawEllipsoidPoints()
    {
        glPointSize(5.0f);
        for(std::size_t p = 0; p < control::AccelerometerCalibration::POINTS_COUNT; ++p)
        {
            if(m_Calibration.isDirectionSampled(p))
            {
                glColor3f(0, 1, 0);
            }
            else
            {
                glColor3f(0.75f, 0.75f, 0.75f);
            }
            
            Vec3f pointDirection(control::AccelerometerCalibration::directionOfIndex(p));
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
