#ifndef CTRLPE_LOCAL_POSE3DVIEW_H
#define CTRLPE_LOCAL_POSE3DVIEW_H

#include "Config.h"

namespace ctrlpe
{
namespace local
{
    class Pose3DView
        : public QWidget
    {
    private:
        
        Q_OBJECT
    protected:
    
    public:
        Pose3DView(QWidget *parent);
        ~Pose3DView();
        
        void setUQuadAttitude(QQuaternion quat);
        
    private:
        void addAxes(Qt3D::QEntity *entity);
        
        
    private:
        QHBoxLayout *m_Layout;
        QWindow *m_Window;
        Qt3D::QAspectEngine *m_AspectEngine;
        Qt3D::QRenderAspect *m_RenderAspect;
        Qt3D::QInputAspect *m_InputAspect;
        Qt3D::QEntity *m_RootEntity;
        Qt3D::QCamera *m_Camera;
        Qt3D::QFrameGraph *m_FrameGraph;
        Qt3D::QForwardRenderer *m_ForwardRenderer;
        
        Qt3D::QEntity *m_uQuadEntity;
        Qt3D::QTransform *m_uQuadTransform;
        Qt3D::QMatrixTransform *m_uQuadMatrixTransform;
    };
    
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_POSE3DVIEW_H