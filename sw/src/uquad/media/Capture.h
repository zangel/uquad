#ifndef UQUAD_MEDIA_CAPTURE_H
#define UQUAD_MEDIA_CAPTURE_H

#include "Config.h"
#include "Generator.h"
#include "../hal/Camera.h"

namespace uquad
{
namespace media
{
    class Capture
    	: public Generator
        , protected hal::CameraDelegate
    {
    public:
        friend class CameraOutputPort;
        
        class CameraOutputPort
            : public media::VideoOutputPort
        {
        public:
            CameraOutputPort(Capture &cap);
            ~CameraOutputPort();
            
            intrusive_ptr<Source> getSource() const;
            SampleType getSampleType() const;
            Sample getSample() const;
            float getSampleRate() const;
            Vec2i getVideoSampleSize() const;
            
        private:
            Capture &m_Capture;
        };
        
        
        Capture();
    	~Capture();
        
        bool isValid() const;
        
        system::error_code prepare(asio::yield_context &yctx);
        void unprepare(asio::yield_context &yctx);
        
        system::error_code resume(asio::yield_context &yctx);
        void pause(asio::yield_context &yctx);

        system::error_code run(asio::yield_context &yctx);
        void rest(asio::yield_context &yctx);
        
        std::size_t numOutputPorts() const;
        intrusive_ptr<OutputPort> getOutputPort(std::size_t index) const;
        
        intrusive_ptr<hal::Camera> const& getCamera() const;
        system::error_code setCamera(intrusive_ptr<hal::Camera> const &camera);
        
    protected:
        SampleType getCameraSampleType() const;
        Sample getCameraSample() const;
        float getCameraSampleRate() const;
        Vec2i getCameraVideoSampleSize() const;
        
    protected:
        void onCameraParametersChanged();
        void onCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer);
        
    private:
        mutable CameraOutputPort m_CameraOutputPort;
        intrusive_ptr<hal::Camera> m_Camera;
        fast_mutex m_VideoBufferQueueSync;
        circular_buffer< intrusive_ptr<base::PixelSampleBuffer> > m_VideoBufferQueue;
        intrusive_ptr<base::PixelSampleBuffer> m_VideoBufferCurrent;
        chrono::system_clock::time_point m_FirstSampleTime;
        float m_SampleTime;
        

    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_CAPTURE_H
