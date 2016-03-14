#include "PoseView.h"

namespace ctrlpe
{
namespace local
{
    PoseView::PoseView(QWidget *parent)
        : View3D(parent)
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
                
        glLineWidth(3);
        glColor3f(0.85f, 0.85f, 0.85f);
        glPushMatrix();
        glTranslatef(-400.0e-3f, 400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef( 400.0e-3f, 400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glPopMatrix();
        
        glColor3f(0.85f, 0.043f, 0.0f);
        glPushMatrix();
        glTranslatef(-400.0e-3f,-400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef( 400.0e-3f,-400.0e-3f, 0.0f);
        drawXYCircle(300.0e-3f);
        glPopMatrix();
        
        
        glPopMatrix();
    }

    
} //namespace local
} //namespace ctrlpe