#ifndef CVTRACKER_LOCAL_CAMERA_VIEW_H
#define CVTRACKER_LOCAL_CAMERA_VIEW_H

#include "Config.h"
#include <uquad/comm/RemoteControl.h>
#include <uquad/base/PixelSampleBuffer.h>
#include <uquad/media/Playout.h>
#include "../common/msg/TrackerInfo.h"


namespace cvtracker
{
namespace local
{
    class CameraView
        : public QOpenGLWidget
        , protected QOpenGLFunctions
        , protected media::PlayoutDelegate
        , protected comm::RemoteControlDelegate
    {
        Q_OBJECT

    public:
        CameraView(QWidget *parent = 0);
        ~CameraView();
        
        Vec2i getVideoSize() const;
        
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int w, int h);
        
        void drawVideoTexture();
        void drawTrackerInfo();
        
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        void onPlayoutVideoResized(Vec2i const &size);
        void onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
        void updateVideoTexture(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
    private:
        mutable mutex m_Guard;
        
        Vec2i m_CameraTextureSize;
        
        void *m_CGLContextObj;
        GLuint m_GLCameraTexture;
        
        intrusive_ptr<common::msg::TrackerInfo> m_TrackerInfo;
    };

} //namespace local
} //namespace cvtracker


#endif //CVTRACKER_LOCAL_CAMERA_VIEW_H