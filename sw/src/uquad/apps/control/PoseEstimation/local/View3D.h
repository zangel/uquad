#ifndef CTRLPE_LOCAL_VIEW3D_H
#define CTRLPE_LOCAL_VIEW3D_H

#include "Config.h"

namespace ctrlpe
{
namespace local
{
    class View3D
        : public QOpenGLWidget
        , protected QOpenGLFunctions
    {
    private:
        
        Q_OBJECT
    protected:
    
    public:
        View3D(QWidget *parent = 0);
        ~View3D();
        
        
        void setCameraPosition(QVector3D p);
        void setCameraLookAt(QVector3D p);
        void setCameraUp(QVector3D p);
        
        void drawAxes(float length = 1.0f);
        void drawXYGrid(QVector2D size = QVector2D(1.0f, 1.0f), QSize segments = QSize(10, 10));
        void drawXYCircle(float radius = 1.0f, std::size_t slices = 36);
        void drawVector3D(QVector3D vec);
        void drawSphere(float radius = 1.0f, std::size_t rings = 15, std::size_t slices = 36);
        
        
        
    protected:
        void initializeGL();
        void resizeGL(int w, int h);
        void paintGL();
        
        void requestDraw();
        
        void mousePressEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        
        virtual void drawView() = 0;
        
        
        
    protected:
        mutable mutex m_Guard;
        
        QMatrix4x4 m_Projection;
        
        QVector3D m_CameraPosition;
        QVector3D m_CameraLookAt;
        QVector3D m_CameraUp;
        QQuaternion m_Rotation;
        QQuaternion m_CurrentRotation;
        
        uquad::math::geom::ArcBall m_ArcBall;
    };
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_VIEW3D_H