#include "Capture.h"
#include "../base/Error.h"
#include "../utility/Constant.h"
#include "Log.h"


namespace uquad
{
namespace media
{
    Capture::CameraOutputPort::CameraOutputPort(Capture &cap)
        : VideoOutputPort()
        , m_Capture(cap)
    {
    }
    
    Capture::CameraOutputPort::~CameraOutputPort()
    {
    }
    
    intrusive_ptr<Source> Capture::CameraOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(const_cast<Capture*>(&m_Capture));
    }
    
    SampleType Capture::CameraOutputPort::getSampleType() const
    {
        return m_Capture.getCameraSampleType();
    }
    
    Sample Capture::CameraOutputPort::getSample() const
    {
        return m_Capture.getCameraSample();
    }
    
    float Capture::CameraOutputPort::getSampleRate() const
    {
        return m_Capture.getCameraSampleRate();
    }
    
    Vec2i Capture::CameraOutputPort::getVideoSampleSize() const
    {
        return m_Capture.getCameraVideoSampleSize();
    }
    
	Capture::Capture()
        : Generator()
        , m_CameraOutputPort(*this)
        , m_Camera()
        , m_VideoBufferQueueSync()
        , m_VideoBufferQueue(2)
        , m_VideoBufferCurrent()
        , m_FirstSampleTime(chrono::system_clock::time_point::max())
        , m_SampleTime(0.0f)
	{
        intrusive_ptr_add_ref(&m_CameraOutputPort);
	}

	Capture::~Capture()
	{
	}
    
    bool Capture::isValid() const
    {
        return Generator::isValid() && (m_Camera.get() != 0);
    }
    
    system::error_code Capture::prepare(asio::yield_context &yctx)
    {
        if(system::error_code e = Generator::prepare(yctx))
            return e;
        
        if(!isValid())
            return base::makeErrorCode(base::kENotApplicable);
        
        //if(m_Camera && !m_Camera->isPreviewStarted())
        //    return base::makeErrorCode(base::kENotApplicable);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void Capture::unprepare(asio::yield_context &yctx)
    {

        Generator::unprepare(yctx);
    }
    
    system::error_code Capture::resume(asio::yield_context &yctx)
    {
        if(system::error_code ge = Generator::resume(yctx))
            return ge;

        {
            lock_guard<fast_mutex> lock(m_VideoBufferQueueSync);
            m_VideoBufferQueue.clear();
        }
        m_VideoBufferCurrent.reset();

        m_FirstSampleTime = chrono::system_clock::time_point::max();
        m_SampleTime = 0.0f;

        return base::makeErrorCode(base::kENoError);
    }

    void Capture::pause(asio::yield_context &yctx)
    {
        {
            lock_guard<fast_mutex> lock(m_VideoBufferQueueSync);
            m_VideoBufferQueue.clear();
        }
        m_VideoBufferCurrent.reset();

        Generator::pause(yctx);
    }


    system::error_code Capture::run(asio::yield_context &yctx)
    {
        if(system::error_code ge = Generator::run(yctx))
            return ge;
        
        {
            lock_guard<fast_mutex> lock(m_VideoBufferQueueSync);
            if(!m_VideoBufferQueue.empty())
            {
                m_VideoBufferCurrent = m_VideoBufferQueue.front();
                m_VideoBufferQueue.pop_front();
            }
        }
        
        if(!m_VideoBufferCurrent)
        {
            UQUAD_MEDIA_LOG(Warning) << "Capture::run: m_VideoBufferCurrent is null";
            return base::makeErrorCode(base::kENoError);
        }
        
        chrono::system_clock::time_point const now =  chrono::system_clock::now();
        m_FirstSampleTime = std::min(now, m_FirstSampleTime);
        m_SampleTime = 1.0e-6f*chrono::duration_cast<chrono::microseconds>(now - m_FirstSampleTime).count();
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void Capture::rest(asio::yield_context &yctx)
    {
        m_VideoBufferCurrent.reset();
        Generator::rest(yctx);
    }
    
    std::size_t Capture::numOutputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<OutputPort> Capture::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_CameraOutputPort);
        
        return intrusive_ptr<OutputPort>();
    }
    
    intrusive_ptr<hal::Camera> const& Capture::getCamera() const
    {
        return m_Camera;
    }
    
    system::error_code  Capture::setCamera(intrusive_ptr<hal::Camera> const &camera)
    {
        if(isPrepared())
            return base::makeErrorCode(base::kENotApplicable);
        
        if(m_Camera)
        {
            m_Camera->cameraDelegate() -= this;
        }
        
        m_Camera = camera;
        
        if(m_Camera)
        {
            m_Camera->cameraDelegate() += this;
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    SampleType Capture::getCameraSampleType() const
    {
        if(!m_Camera)
            return SampleType();
        
        return SampleType(m_Camera->getVideoFormat());
    }
    
    Sample Capture::getCameraSample() const
    {
        if(!m_VideoBufferCurrent)
            return Sample();
        
        return Sample(m_VideoBufferCurrent, m_SampleTime);
    }
    
    float Capture::getCameraSampleRate() const
    {
        if(!m_Camera)
            return 0.0f;
        
        return m_Camera->getPreviewFPS();
    }
    
    Vec2i Capture::getCameraVideoSampleSize() const
    {
        if(!m_Camera)
            return utility::Constant<Vec2i>::value();
        
        return m_Camera->getVideoSize();
    }
    
    void Capture::onCameraParametersChanged()
    {
        
    }
    
    void Capture::onCameraVideoFrame(intrusive_ptr<base::PixelSampleBuffer> const &buffer)
    {
        if(!isRunning())
            return;
        
        bool shouldNotify = false;
        {
            lock_guard<fast_mutex> lock(m_VideoBufferQueueSync);
            shouldNotify = !m_VideoBufferQueue.full();
            m_VideoBufferQueue.push_back(buffer);
        }
        
        if(shouldNotify)
        {
            notifyOnSampleGenerated();
        }
    }

} //namespace media
} //namespace uquad
