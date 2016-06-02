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
        void setUQuadAttitudeSetpoint(QQuaternion quat);
        void setMagneticField(QVector3D vec);
        void setMotorsPower(QVector4D mp);
        
    protected:
        void drawView();
        void drawUQuad();
        void drawUQuadSetpoint();
        
    private:
        QQuaternion m_UQuadAttitude;
        QQuaternion m_UQuadAttitudeSetpoint;
        QVector3D m_MagneticField;
        QVector4D m_MotorsPower;
        
    };
    
    
} //namespace local
} //namespace ctrlpe

#endif //CTRLPE_LOCAL_POSEVIEW_H