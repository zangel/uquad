#ifndef CVCC_LOCAL_CAMERA_VIEW_H
#define CVCC_LOCAL_CAMERA_VIEW_H

#include "Config.h"
#include <uquad/comm/CameraClient.h>
#include <uquad/media/Playout.h>
#include <uquad/base/PixelSampleBuffer.h>


namespace cvcc
{
namespace local
{
    class CameraView
        : public QOpenGLWidget
        , protected QOpenGLFunctions
        , protected comm::CameraClientDelegate
        , protected media::PlayoutDelegate
    {
        Q_OBJECT

    public:
        CameraView(QWidget *parent = 0);
        ~CameraView();
        
        cv::Size getVideoSize() const;
        cv::Size getChessboardSize() const;
        bool getChessboardCorners(std::vector<cv::Point2f> &cc) const;
        
    protected:
        void initializeGL();
        void paintGL();
        void resizeGL(int w, int h);
        
        void drawVideoTexture();
        void drawChessboard();
        
        void onPlayoutVideoResized(Vec2i const &size);
        void onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
        void updateVideoTexture(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        void detectChessboardCorners(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
    private:
        mutable mutex m_Guard;
        
        Vec2i m_CameraTextureSize;
        cv::Mat m_LastView;
        
        bool m_ChessboardCornersFound;
        std::vector<cv::Point2f> m_ChessboardCorners;
        
        void *m_CGLContextObj;
        GLuint m_GLCameraTexture;
    };
    
} //namespace local
} //namespace cvcc

#endif //CVCC_LOCAL_CAMERA_VIEW_H