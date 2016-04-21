#ifndef CTRLPE_LOCAL_POSEVIEW_H
#define CTRLPE_LOCAL_POSEVIEW_H

#include "View3D.h"

namespace ctrlpe
{
namespace local
{
    class PoseView
        : public View3D
    {
    private:
        
        Q_OBJECT
    protected:
    
    public:
        PoseView(QWidget *parent = 0);
        ~PoseView();
        
        void setUQuadAttitude(QQuaternion quat);
        void setMagneticField(QVector3D vec);
        void setMotorsThrust(QVector4D mt);
        
    protected:
        void drawView();
        
    private:
        QQuaternion m_UQuadAttitude;
        QVector3D m_MagneticField;
        QVector4D m_MotorsThrust;
        
    };
    
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_POSEVIEW_H