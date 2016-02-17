#include "CameraView.h"
#include "Application.h"
#include <OpenGL/OpenGL.h>


namespace cvtracker
{
namespace local
{
    CameraView::CameraView(QWidget *parent)
        : QOpenGLWidget(parent)
        , m_Guard()
        , m_CGLContextObj(0)
        , m_GLCameraTexture(0)
    {
        Application::instance()->getRemoteControl().remoteControlDelegate() += this;
        Application::instance()->getCameraClient().getPlayout()->playoutDelegate()+=this;
    }
    
    CameraView::~CameraView()
    {
        Application::instance()->getCameraClient().getPlayout()->playoutDelegate()-=this;
        Application::instance()->getRemoteControl().remoteControlDelegate() -= this;
    }
    
    Vec2i CameraView::getVideoSize() const
    {
        return m_CameraTextureSize;
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
        drawTrackerInfo();
    }
    
    void CameraView::resizeGL(int w, int h)
    {
        lock_guard<mutex> lock(m_Guard);
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
    
    void CameraView::drawTrackerInfo()
    {
        if(!m_TrackerInfo)
            return;
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        if(!m_TrackerInfo->trackingFeatures.empty())
        {
            glPointSize(1.5f);
            glBegin(GL_POINTS);
            glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
            
            for(std::size_t f = 0; f < m_TrackerInfo->trackingFeatures.size(); ++f)
            {
                glVertex2f(
                    -1.0f + 2.0f*m_TrackerInfo->trackingFeatures[f][0]/m_CameraTextureSize[0],
                     1.0f - 2.0f*m_TrackerInfo->trackingFeatures[f][1]/m_CameraTextureSize[1]
                );
            }
            glEnd();
        }
        
        if(!m_TrackerInfo->trackingTrails.empty())
        {
            glPointSize(5);
            glLineWidth(2);
            glEnable(GL_POINT_SMOOTH);
            glEnable(GL_LINE_SMOOTH);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);
            glBegin(GL_LINES);
            
            for(std::size_t t = 0; t < m_TrackerInfo->trackingTrails.size(); ++t)
            {
                glColor3f(1,1,0);
                glVertex2f(
                    -1.0f + 2.0f*m_TrackerInfo->trackingTrails[t].first[0]/m_CameraTextureSize[0],
                     1.0f - 2.0f*m_TrackerInfo->trackingTrails[t].first[1]/m_CameraTextureSize[1]
                );
                
                glColor3f(1,0,0);
                glVertex2f(
                    -1.0f + 2.0f*m_TrackerInfo->trackingTrails[t].second[0]/m_CameraTextureSize[0],
                     1.0f - 2.0f*m_TrackerInfo->trackingTrails[t].second[1]/m_CameraTextureSize[1]
                );
            }
            
            glEnd();
        }
        
        std::size_t const refGridHSize = m_TrackerInfo->referenceGridHalfSize;
        std::size_t const refGridSize  = refGridHSize * 2 + 1;
        if(!m_TrackerInfo->referenceGrid.empty())
        {
            glColor4f(0, 0, 0, 0.6);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glLineWidth(2);
            
            
            for(std::size_t i = 0; i < refGridSize; i++)
            {
                glBegin(GL_LINE_STRIP);
                for(std::size_t j = 0; j < refGridSize; j++)
                {
                    glVertex2f(
                        -1.0f + 2.0f*m_TrackerInfo->referenceGrid[i*refGridSize+j][0]/m_CameraTextureSize[0],
                         1.0f - 2.0f*m_TrackerInfo->referenceGrid[i*refGridSize+j][1]/m_CameraTextureSize[1]
                    );
                }
                glEnd();
        
                glBegin(GL_LINE_STRIP);
                for(std::size_t j = 0; j < refGridSize; j++)
                {
                    glVertex2f(
                        -1.0f + 2.0f*m_TrackerInfo->referenceGrid[j*refGridSize+i][0]/m_CameraTextureSize[0],
                         1.0f - 2.0f*m_TrackerInfo->referenceGrid[j*refGridSize+i][1]/m_CameraTextureSize[1]
                    );
                }
                glEnd();
            }
        }
    }
    
    void CameraView::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
        if(intrusive_ptr<common::msg::TrackerInfo> msgTrackerInfo = dynamic_pointer_cast<common::msg::TrackerInfo>(msg))
        {
            lock_guard<mutex> lock(m_Guard);
            m_TrackerInfo = msgTrackerInfo;
        }
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

} //namespace local
} //namespace cvtracker

