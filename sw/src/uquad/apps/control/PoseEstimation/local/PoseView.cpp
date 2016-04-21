#include "PoseView.h"

namespace ctrlpe
{
namespace local
{
    PoseView::PoseView(QWidget *parent)
        : View3D(parent)
        , m_UQuadAttitude()
        , m_MagneticField()
        , m_MotorsThrust()
    {
    }
    
    PoseView::~PoseView()
    {
    }
    
    void PoseView::setUQuadAttitude(QQuaternion quat)
    {
        m_UQuadAttitude = quat;
        requestDraw();
    }
    
    void PoseView::setMagneticField(QVector3D vec)
    {
        m_MagneticField = vec;
        requestDraw();
    }
    
    void PoseView::setMotorsThrust(QVector4D mt)
    {
        m_MotorsThrust = mt;
        requestDraw();
    }
    
    void PoseView::drawView()
    {
        drawXYGrid(QVector2D(2, 2));
        drawAxes();
        
        QMatrix4x4 uquadAttitude;
        uquadAttitude.rotate(m_UQuadAttitude);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixf(uquadAttitude.constData());
        
        drawAxes();
        
        glLineWidth(3);
        glColor3f(0.95f, 0.92f, 0.0f);
        drawVector3D(m_MagneticField);
                
        glPushMatrix();
        glLineWidth(3);
        glColor3f(0.85f, 0.85f, 0.85f);
        glTranslatef(-400.0e-3f, 400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glLineWidth(3);
        glColor3f(0.72f, 0.54f, 0.0f);
        drawVector3D(QVector3D(0.0f, 0.0f, 0.5f*m_MotorsThrust[0]));
        glPopMatrix();
        
        glPushMatrix();
        glLineWidth(3);
        glColor3f(0.85f, 0.85f, 0.85f);
        glTranslatef( 400.0e-3f, 400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glLineWidth(3);
        glColor3f(0.72f, 0.54f, 0.0f);
        drawVector3D(QVector3D(0.0f, 0.0f, 0.5f*m_MotorsThrust[1]));
        glPopMatrix();
        
        glPushMatrix();
        glLineWidth(3);
        glColor3f(0.85f, 0.043f, 0.0f);
        glTranslatef(-400.0e-3f,-400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glLineWidth(3);
        glColor3f(0.72f, 0.54f, 0.0f);
        drawVector3D(QVector3D(0.0f, 0.0f, 0.5f*m_MotorsThrust[3]));
        glPopMatrix();
        
        glPushMatrix();
        glLineWidth(3);
        glColor3f(0.85f, 0.043f, 0.0f);
        glTranslatef( 400.0e-3f,-400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glLineWidth(3);
        glColor3f(0.72f, 0.54f, 0.0f);
        drawVector3D(QVector3D(0.0f, 0.0f, 0.5f*m_MotorsThrust[2]));
        glPopMatrix();
        
        
        glPopMatrix();
    }

    
} //namespace local
} //namespace ctrlpe