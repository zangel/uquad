#include "Pose3DView.h"

namespace ctrlpe
{
namespace local
{
    Pose3DView::Pose3DView(QWidget *parent)
        : QWidget(parent)
    {
        m_Layout = new QHBoxLayout(this);
        m_Layout->setContentsMargins(0, 0, 0, 0);
        
        m_Window = new QWindow();
        m_Window->setSurfaceType(QSurface::OpenGLSurface);
        QSurfaceFormat format;
        if(QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
        {
            format.setVersion(4, 3);
            format.setProfile(QSurfaceFormat::CoreProfile);
        }
        format.setDepthBufferSize(24);
        format.setSamples( 4 );
        format.setStencilBufferSize(8);
        m_Window->setFormat(format);
        m_Window->create();
        
        QWidget *container = QWidget::createWindowContainer(m_Window);
        m_Layout->addWidget(container, 1);
        
        m_AspectEngine = new Qt3D::QAspectEngine();
        m_RenderAspect = new Qt3D::QRenderAspect();
        m_InputAspect = new Qt3D::QInputAspect();
        
        m_AspectEngine->registerAspect(m_RenderAspect);
        m_AspectEngine->registerAspect(m_InputAspect);
        m_AspectEngine->initialize();
        
        QVariantMap data;
        data.insert(QStringLiteral("surface"), QVariant::fromValue(static_cast<QSurface *>(m_Window)));
        data.insert(QStringLiteral("eventSource"), QVariant::fromValue(m_Window));
        m_AspectEngine->setData(data);
        
        m_RootEntity = new Qt3D::QEntity();
        m_Camera = new Qt3D::QCamera(m_RootEntity);
        m_Camera->setObjectName(QStringLiteral("Camera"));
        m_Camera->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 1.0e-3f, 100.0f);
        m_Camera->setPosition(QVector3D(0, -0.5f, 0.5f));
        m_Camera->setUpVector(QVector3D(0, 0, 1));
        m_Camera->setViewCenter(QVector3D(0, 0, 0));
        m_InputAspect->setCamera(m_Camera);
        
        
        m_FrameGraph = new Qt3D::QFrameGraph();
        
        m_ForwardRenderer = new Qt3D::QForwardRenderer();
        
        m_ForwardRenderer->setCamera(m_Camera);
        m_ForwardRenderer->setClearColor(Qt::gray);
        m_FrameGraph->setActiveFrameGraph(m_ForwardRenderer);

        m_RootEntity->addComponent(m_FrameGraph);

        m_AspectEngine->setRootEntity(m_RootEntity);
        
        #if 0
        {
            Qt3D::QPhongMaterial *xyPlaneMaterial = new Qt3D::QPhongMaterial();
            xyPlaneMaterial->setAmbient(Qt::black);
            QColor xyPlaneColor = Qt::gray;
            xyPlaneColor.setAlphaF(0.1f);
            xyPlaneMaterial->setDiffuse(xyPlaneColor);
            xyPlaneMaterial->setSpecular(Qt::black);
            
            Qt3D::QEntity *xyPlaneEntity = new Qt3D::QEntity(m_RootEntity);
            
            Qt3D::QPlaneMesh *xyPlaneMesh = new Qt3D::QPlaneMesh();
            Qt3D::QTransform *xyPlaneTransform = new Qt3D::QTransform();
            Qt3D::QRotateTransform *xyPlaneRotationTransform = new Qt3D::QRotateTransform();
            Qt3D::QTranslateTransform *xyPlaneTralsateTransform = new Qt3D::QTranslateTransform();
            
            xyPlaneMesh->setMeshResolution(QSize(10, 10));
            xyPlaneMesh->setWidth(10.0f);
            xyPlaneMesh->setHeight(10.0f);
            
            xyPlaneRotationTransform->setAngleDeg(90.0f);
            xyPlaneRotationTransform->setAxis(QVector3D(1, 0, 0));
    
            xyPlaneTransform->addTransform(xyPlaneRotationTransform);
            xyPlaneTransform->addTransform(xyPlaneTralsateTransform);
            
            xyPlaneEntity->addComponent(xyPlaneMesh);
            xyPlaneEntity->addComponent(xyPlaneTransform);
            xyPlaneEntity->addComponent(xyPlaneMaterial);
        }
        #endif

        
        {
            Qt3D::QPhongMaterial *frontMaterial = new Qt3D::QPhongMaterial();
            frontMaterial->setAmbient(Qt::black);
            frontMaterial->setDiffuse(Qt::white);
            frontMaterial->setSpecular(Qt::black);
            //frontMaterial->setShininess(150.0f);
            
    
            Qt3D::QPhongMaterial *backMaterial = new Qt3D::QPhongMaterial();
            backMaterial->setAmbient(Qt::black);
            backMaterial->setDiffuse(Qt::red);
            backMaterial->setSpecular(Qt::black);
            //backMaterial->setShininess(150.0f);
            
            
            // motor shape
            Qt3D::QTorusMesh *motorMesh = new Qt3D::QTorusMesh();
            motorMesh->setRadius(40.0e-3f);
            motorMesh->setMinorRadius(5.0e-3f);
            
            m_uQuadEntity = new Qt3D::QEntity(m_RootEntity);
            m_uQuadTransform = new Qt3D::QTransform();
            m_uQuadMatrixTransform = new Qt3D::QMatrixTransform();
            m_uQuadTransform->addTransform(m_uQuadMatrixTransform);
            m_uQuadEntity->addComponent(m_uQuadTransform);
            
            
            // motor0
            Qt3D::QEntity *motor0 = new Qt3D::QEntity(m_uQuadEntity);
            motor0->addComponent(motorMesh);
            Qt3D::QTranslateTransform *motor0Translation = new Qt3D::QTranslateTransform();
            Qt3D::QTransform *motor0Transforms = new Qt3D::QTransform();
            
            motor0Translation->setTranslation(QVector3D(-50.0e-3f, 50.0e-3f, 0.0f));
            motor0Transforms->addTransform(motor0Translation);
            motor0->addComponent(motor0Transforms);
            motor0->addComponent(frontMaterial);
            
            
            // motor1
            Qt3D::QEntity *motor1 = new Qt3D::QEntity(m_uQuadEntity);
            motor1->addComponent(motorMesh);
            Qt3D::QTranslateTransform *motor1Translation = new Qt3D::QTranslateTransform();
            Qt3D::QTransform *motor1Transforms = new Qt3D::QTransform();
            
            motor1Translation->setTranslation(QVector3D(50.0e-3f, 50.0e-3f, 0.0f));
            motor1Transforms->addTransform(motor1Translation);
            motor1->addComponent(motor1Transforms);
            motor1->addComponent(frontMaterial);
            
            // motor2
            Qt3D::QEntity *motor2 = new Qt3D::QEntity(m_uQuadEntity);
            motor2->addComponent(motorMesh);
            Qt3D::QTranslateTransform *motor2Translation = new Qt3D::QTranslateTransform();
            Qt3D::QTransform *motor2Transforms = new Qt3D::QTransform();
            
            motor2Translation->setTranslation(QVector3D(-50.0e-3f, -50.0e-3f, 0.0f));
            motor2Transforms->addTransform(motor2Translation);
            motor2->addComponent(motor2Transforms);
            motor2->addComponent(backMaterial);
            
            // motor3
            Qt3D::QEntity *motor3 = new Qt3D::QEntity(m_uQuadEntity);
            motor3->addComponent(motorMesh);
            Qt3D::QTranslateTransform *motor3Translation = new Qt3D::QTranslateTransform();
            Qt3D::QTransform *motor3Transforms = new Qt3D::QTransform();
            
            motor3Translation->setTranslation(QVector3D(50.0e-3f, -50.0e-3f, 0.0f));
            motor3Transforms->addTransform(motor3Translation);
            motor3->addComponent(motor3Transforms);
            motor3->addComponent(backMaterial);
        }
        
        addAxes(m_uQuadEntity);
        addAxes(m_RootEntity);
    }
    
    Pose3DView::~Pose3DView()
    {
    }
    
    void Pose3DView::setUQuadAttitude(QQuaternion quat)
    {
        QMatrix4x4 mat;
        mat.rotate(quat);
        m_uQuadMatrixTransform->setMatrix(mat);
    }
    
    void Pose3DView::addAxes(Qt3D::QEntity *entity)
    {
        Qt3D::QPhongMaterial *axisXMaterial = new Qt3D::QPhongMaterial();
        axisXMaterial->setAmbient(Qt::black);
        axisXMaterial->setDiffuse(Qt::red);
        axisXMaterial->setSpecular(Qt::black);
        
        Qt3D::QPhongMaterial *axisYMaterial = new Qt3D::QPhongMaterial();
        axisYMaterial->setAmbient(Qt::black);
        axisYMaterial->setDiffuse(Qt::green);
        axisYMaterial->setSpecular(Qt::black);
        
        Qt3D::QPhongMaterial *axisZMaterial = new Qt3D::QPhongMaterial();
        axisZMaterial->setAmbient(Qt::black);
        axisZMaterial->setDiffuse(Qt::blue);
        axisZMaterial->setSpecular(Qt::black);
        
        Qt3D::QCylinderMesh *axisMesh = new Qt3D::QCylinderMesh();
        axisMesh->setRadius(1.0e-3f);
        axisMesh->setLength(0.2f);
        axisMesh->setRings(2);
        axisMesh->setSlices(5);
        
        Qt3D::QEntity *axisX = new Qt3D::QEntity(entity);
        Qt3D::QTranslateTransform *axisXTranslation = new Qt3D::QTranslateTransform();
        Qt3D::QRotateTransform *axisXRotation = new Qt3D::QRotateTransform();
        Qt3D::QTransform *axisXTransform = new Qt3D::QTransform();
        axisXTranslation->setTranslation(QVector3D(0.1f, 0.0f, 0.0f));
        axisXRotation->setAngleDeg(-90.0f);
        axisXRotation->setAxis(QVector3D(0, 0, 1));
        axisXTransform->addTransform(axisXRotation);
        axisXTransform->addTransform(axisXTranslation);
        axisX->addComponent(axisMesh);
        axisX->addComponent(axisXTransform);
        axisX->addComponent(axisXMaterial);
        
        Qt3D::QEntity *axisY = new Qt3D::QEntity(entity);
        Qt3D::QTranslateTransform *axisYTranslation = new Qt3D::QTranslateTransform();
        Qt3D::QTransform *axisYTransform = new Qt3D::QTransform();
        axisYTranslation->setTranslation(QVector3D(0.0f, 0.1f, 0.0f));
        axisYTransform->addTransform(axisYTranslation);
        axisY->addComponent(axisMesh);
        axisY->addComponent(axisYTransform);
        axisY->addComponent(axisYMaterial);
        
        Qt3D::QEntity *axisZ = new Qt3D::QEntity(entity);
        Qt3D::QTranslateTransform *axisZTranslation = new Qt3D::QTranslateTransform();
        Qt3D::QRotateTransform *axisZRotation = new Qt3D::QRotateTransform();
        Qt3D::QTransform *axisZTransform = new Qt3D::QTransform();
        axisZTranslation->setTranslation(QVector3D(0.0f, 0.0f, 0.1f));
        axisZRotation->setAngleDeg(-90.0f);
        axisZRotation->setAxis(QVector3D(1, 0, 0));
        axisZTransform->addTransform(axisZRotation);
        axisZTransform->addTransform(axisZTranslation);
        axisZ->addComponent(axisMesh);
        axisZ->addComponent(axisZTransform);
        axisZ->addComponent(axisZMaterial);
    }

} //namespace local
} //namespace ctrlpe