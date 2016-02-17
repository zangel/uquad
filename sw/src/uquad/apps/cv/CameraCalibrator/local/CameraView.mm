#include "CameraView.h"
#include "Application.h"

#include <OpenGL/OpenGL.h>

//#include <uquad/hal/av/CVImagePixelSampleBuffer.h>
//#include <OpenGL/CGLIOSurface.h>


namespace cvcc
{
namespace local
{
    CameraView::CameraView(QWidget *parent)
        : QOpenGLWidget(parent)
        , m_Guard()
        , m_ChessboardCornersFound(false)
        , m_ChessboardCorners()
        , m_CGLContextObj(0)
        , m_GLCameraTexture(0)
    {
        Application::instance()->getCameraClient().cameraClientDelegate()+=this;
        Application::instance()->getCameraClient().getPlayout()->playoutDelegate()+=this;
    }
    
    CameraView::~CameraView()
    {
        Application::instance()->getCameraClient().getPlayout()->playoutDelegate()-=this;
        Application::instance()->getCameraClient().cameraClientDelegate()-=this;
    }
    
    cv::Size CameraView::getVideoSize() const
    {
        return cv::Size(m_CameraTextureSize[0], m_CameraTextureSize[1]);
    }
    
    cv::Size CameraView::getChessboardSize() const
    {
        return cv::Size(11,7);
    }
    
    bool CameraView::getChessboardCorners(std::vector<cv::Point2f> &cc) const
    {
        lock_guard<mutex> lock(m_Guard);
        if(!m_ChessboardCornersFound || m_ChessboardCorners.empty())
            return false;
        
        cc.assign(m_ChessboardCorners.begin(), m_ChessboardCorners.end());
        
        return true;
    }
    
    void CameraView::initializeGL()
    {
        lock_guard<mutex> lock(m_Guard);
        initializeOpenGLFunctions();
        glClearColor(0, 0, 0, 1);
        glGenTextures(1, &m_GLCameraTexture);
        m_CGLContextObj = CGLGetCurrentContext();
    }
    
    void CameraView::paintGL()
    {
        lock_guard<mutex> lock(m_Guard);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        drawVideoTexture();
        drawChessboard();
    }
    
    void CameraView::drawVideoTexture()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glBindTexture(GL_TEXTURE_2D, m_GLCameraTexture); // Grab the texture from the delegate
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        glBegin(GL_QUADS);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(-1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(1.0f, -1.0f);

        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(1.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(-1.0f, 1.0f);

        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    
    void CameraView::drawChessboard()
    {
        if(!m_ChessboardCornersFound || m_ChessboardCorners.empty())
            return;
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        glPointSize(5);
        glBegin(GL_POINTS);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        for(std::size_t i = 0; i < m_ChessboardCorners.size(); ++i)
        {
            glVertex2f(
                -1.0f + 2.0f*m_ChessboardCorners[i].x/m_CameraTextureSize[0],
                1.0f - 2.0f*m_ChessboardCorners[i].y/m_CameraTextureSize[1]
            );
        }
        
        glEnd();
    }
    
    void CameraView::resizeGL(int w, int h)
    {
        lock_guard<mutex> lock(m_Guard);
    }
    
    void CameraView::onPlayoutVideoResized(Vec2i const &size)
    {
        lock_guard<mutex> lock(m_Guard);
        m_CameraTextureSize = size;
    }
    
    void CameraView::onPlayoutVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        lock_guard<mutex> lock(m_Guard);
        
        if(buffer->lock())
            return;
        
        updateVideoTexture(buffer);
        detectChessboardCorners(buffer);
        
        update();
        
        buffer->unlock();
    }
    
    void CameraView::updateVideoTexture(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        /*intrusive_ptr<hal::av::CVImagePixelSampleBuffer> cvImgSampleBuffer = dynamic_pointer_cast<hal::av::CVImagePixelSampleBuffer>(buffer);
        if(!cvImgSampleBuffer)
            return;
        
        CVPixelBufferRef pixelBuffer = cvImgSampleBuffer->imageBuffer();
        IOSurfaceRef pixelBufferSurface = CVPixelBufferGetIOSurface(pixelBuffer);
        
        */
        
        CGLContextObj cglContext = reinterpret_cast<CGLContextObj>(m_CGLContextObj);
        CGLLockContext(cglContext);
        CGLSetCurrentContext(cglContext);
        
        Vec2i bufferSize = buffer->planeSize();
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_GLCameraTexture);
        
        
        
        /*
        #if 0
        CGLError status = CGLTexImageIOSurface2D(
            cglContext,                             // ctx
            GL_TEXTURE_RECTANGLE_ARB,               // target
            GL_RGB,                                 // internal_format
            bufferSize[0],                          // width
            bufferSize[1],                          // height
            GL_YCBCR_422_APPLE,                     // format
            GL_UNSIGNED_SHORT_8_8_APPLE,            // type
            pixelBufferSurface,                     // io_surface
            0
        );
        #else
        CGLError status = CGLTexImageIOSurface2D(
            cglContext,                             // ctx
            GL_TEXTURE_RECTANGLE_ARB,               // target
            GL_RGB,                                 // internal_format
            bufferSize[0],                          // width
            bufferSize[1],                          // height
            GL_BGRA,                                // format
            GL_UNSIGNED_INT_8_8_8_8_REV,            // type
            pixelBufferSurface,                     // io_surface
            0
        );
        
        if(status)
        {
            NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(status), glGetError());
        }
        #endif
        */
        
        glTexImage2D(GL_TEXTURE_2D,
            0,
            GL_LUMINANCE, //GL_RGBA,
            bufferSize[0],
            bufferSize[1],
            0,
            GL_LUMINANCE, //GL_BGRA,
            GL_UNSIGNED_BYTE, //GL_UNSIGNED_INT_8_8_8_8_REV,
            buffer->planeData()
        );
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        
        glDisable(GL_TEXTURE_2D);
        
        CGLSetCurrentContext(0);
        CGLUnlockContext(cglContext);
    }
    
    void CameraView::detectChessboardCorners(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        cv::Size boardSize = getChessboardSize();
        
        
        Vec2i bfferSize = buffer->planeSize();
        
        cv::Mat view(
            bfferSize[1],
            bfferSize[0],
            CV_8UC1,
            buffer->planeData(),
            buffer->planeRowBytes()
        );
        
        view.copyTo(m_LastView);
        
        m_ChessboardCornersFound = cv::findChessboardCorners(
            m_LastView,
            boardSize,
            m_ChessboardCorners,
            cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE
        );
        
        if(m_ChessboardCornersFound)
        {
            cv::cornerSubPix(
                m_LastView,
                m_ChessboardCorners,
                cv::Size(11,11),
                cv::Size(-1,-1),
                cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.1)
            );
        }
        
    }

} //namespace local
} //namespace cvcc