#include "View3D.h"

namespace ctrlpe
{
namespace local
{
    View3D::View3D(QWidget *parent)
        : QOpenGLWidget(parent)
        , m_Guard()
        , m_Projection()
        , m_CameraPosition(0, -3.0f, 1.0f)
        , m_CameraLookAt(0.0f, 0.0f, 0.0f)
        , m_CameraUp(0.0f, 0.0f, 1.0f)
        , m_Rotation()
        , m_CurrentRotation()
        , m_ArcBall()
    {
    }
    
    View3D::~View3D()
    {
    }
    
    void View3D::setCameraPosition(QVector3D p)
    {
        m_CameraPosition = p;
    }
    
    void View3D::setCameraLookAt(QVector3D p)
    {
        m_CameraLookAt = p;
    }
    
    void View3D::setCameraUp(QVector3D p)
    {
        m_CameraUp = p;
    }
    
    void View3D::drawAxes(float length)
    {
        glLineWidth(3);
        glColor3f(1, 0, 0);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(length, 0, 0);
        glEnd();
        
        glColor3f(0, 1, 0);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, length, 0);
        glEnd();
        
        glColor3f(0, 0, 1);
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(0, 0, length);
        glEnd();
    }
    
    void View3D::drawXYGrid(QVector2D size, QSize segments)
    {
        float const xStep = size.x()/segments.width();
        float const yStep = size.y()/segments.height();
        
        glLineWidth(1);
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        
        for(std::size_t x = 0; x <= segments.width(); ++x)
        {
            float const xp = -0.5f*size.x() + x * xStep;
            glVertex3f(xp, -0.5f*size.y(), 0.0f);
            glVertex3f(xp,  0.5f*size.y(), 0.0f);
        }
        
        for(std::size_t y = 0; y <= segments.height(); ++y)
        {
            float const yp = -0.5f*size.y() + y * yStep;
            glVertex3f(-0.5f*size.x(), yp, 0.0f);
            glVertex3f( 0.5f*size.x(), yp, 0.0f);
        }
        glEnd();
    }
    
    void View3D::drawXYCircle(float radius, std::size_t slices)
    {
        float const step = 2 * M_PI / slices;
        glBegin(GL_LINE_STRIP);
        for(std::size_t s = 0; s <= slices; ++s)
        {
            glVertex3f(radius * std::cos(s * step), radius * std::sin(s * step), 0.0f);
        }
        glEnd();
    }
    
    void View3D::drawVector3D(QVector3D vec)
    {
        glBegin(GL_LINES);
        glVertex3f(0, 0, 0);
        glVertex3f(vec.x(), vec.y(), vec.z());
        glEnd();
    }
    
    void View3D::drawSphere(float radius, std::size_t rings, std::size_t slices)
    {
        float const ringsStep = M_PI / (rings + 1);
        float const sliceStep = 2 * M_PI / slices;
        
        glBegin(GL_POINTS);
        
        glVertex3f(0, 0,  1);
        glVertex3f(0, 0, -1);
        
        for(std::size_t ring = 0; ring <= rings; ++ring)
        {
            float const ringAngle = M_PI_2 - (ring + 1) * ringsStep;
            float const ringZ = radius * std::sin(ringAngle);
            float const ringR = radius * std::cos(ringAngle);
            for(std::size_t slice = 0; slice < slices; ++slice)
            {
                glVertex3f(
                    ringR * std::cos(slice * sliceStep),
                    ringR * std::sin(slice * sliceStep),
                    ringZ
                );
            }
        }
        
        glEnd();
        
    }
    
    void View3D::initializeGL()
    {
        lock_guard<mutex> lock(m_Guard);
        initializeOpenGLFunctions();
        glClearColor(0.16f, 0.16f, 0.16f, 1);
    }
    
    void View3D::resizeGL(int w, int h)
    {
        lock_guard<mutex> lock(m_Guard);
        float const aspect = float(w) / float(h);
        
        float const zNear = 0.1f, zFar = 1000.0f, fov = 45.0;

        m_Projection.setToIdentity();
        m_Projection.perspective(fov, aspect, zNear, zFar);
        
        m_ArcBall.setBounds(w, h);
    }
    
    void View3D::paintGL()
    {
        lock_guard<mutex> lock(m_Guard);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(m_Projection.constData());
        
        glMatrixMode(GL_MODELVIEW);
        QMatrix4x4 mvMatrix;
        mvMatrix.lookAt(m_CameraPosition, m_CameraLookAt, m_CameraUp);
        mvMatrix.rotate(m_CurrentRotation * m_Rotation);
        
        glLoadMatrixf(mvMatrix.constData());
        
        drawView();
    }
    
    void View3D::requestDraw()
    {
        lock_guard<mutex> lock(m_Guard);
        QMetaObject::invokeMethod(
            this,
            "update",
            Qt::QueuedConnection
        );
        //update();
    }
    
    void View3D::mousePressEvent(QMouseEvent *e)
    {
        QOpenGLWidget::mousePressEvent(e);
        if(e->isAccepted())
            return;

        if(e->buttons() & Qt::LeftButton)
        {
            QPointF p = e->pos();
            m_ArcBall.start(Vec2f(p.x(), p.y()));
            e->accept();
            update();
        }
    }
    
    void View3D::mouseMoveEvent(QMouseEvent *e)
    {
        
        QOpenGLWidget::mouseMoveEvent(e);
        if(e->isAccepted())
            return;
        
        if(e->buttons() & Qt::LeftButton)
        {
            QPointF p = e->pos();
            Quaternionf r = m_ArcBall.move(Vec2f(p.x(), p.y()));
            m_CurrentRotation = QQuaternion(r.w(), r.x(), r.y(), r.z());
            e->accept();
            update();
        }
        
    }
    
    void View3D::mouseReleaseEvent(QMouseEvent *e)
    {
        QOpenGLWidget::mouseReleaseEvent(e);
        if(e->isAccepted())
            return;

        if(e->button() == Qt::LeftButton)
        {
            m_Rotation = m_CurrentRotation * m_Rotation;
            m_CurrentRotation = QQuaternion();
            e->accept();
            update();
        }
    }
    
    

} //namespace local
} //namespace ctrlpe