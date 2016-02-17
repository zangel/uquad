#include "Playout.h"
#include "../base/Error.h"

namespace uquad
{
namespace media
{
    Playout::VideoInputPort::VideoInputPort(Playout &po)
        : media::VideoInputPort()
        , m_Playout(po)
    {
    }
    
    Playout::VideoInputPort::~VideoInputPort()
    {
    }
    
    intrusive_ptr<Sink> Playout::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_Playout);
    }
    
    bool Playout::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoRaw)
            return false;
        
        return true;
    }
    
    Playout::Playout()
        : Consumer()
        , m_VideoInputPort(*this)
        , m_PlayoutDelegate()
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
    }
    
    Playout::~Playout()
    {
    }
    
    bool Playout::isValid() const
    {
        return Consumer::isValid();
    }
    
    system::error_code Playout::prepare(asio::yield_context &yctx)
    {
        if(system::error_code ce = Consumer::prepare(yctx))
            return ce;
        Vec2i videoSampleSize = m_VideoInputPort.getVideoSampleSize();
        notifyOnPlayoutVideoResized(videoSampleSize);
        return base::makeErrorCode(base::kENoError);
    }
    
    void Playout::unprepare(asio::yield_context &yctx)
    {
        Consumer::unprepare(yctx);
    }
        
    system::error_code Playout::run(asio::yield_context &yctx)
    {
        if(system::error_code ee = Consumer::run(yctx))
            return ee;
        
        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
            return base::makeErrorCode(base::kEInvalidContent);

        intrusive_ptr<base::PixelSampleBuffer> pixelSampleBuffer = dynamic_pointer_cast<base::PixelSampleBuffer>(videoSample.buffer);

        if(!pixelSampleBuffer)
            return base::makeErrorCode(base::kEInvalidContent);
        
        
        notifyOnPlayoutVideoFrame(pixelSampleBuffer);
        return base::makeErrorCode(base::kENoError);
    }
    
    void Playout::rest(asio::yield_context &yctx)
    {
        Consumer::rest(yctx);
    }
    
    std::size_t Playout::numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<InputPort> Playout::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);
        
        return intrusive_ptr<InputPort>();
    }
    
    
} //namespace media
} //namespace uquad
