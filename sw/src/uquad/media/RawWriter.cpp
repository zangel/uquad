#include "RawWriter.h"
#include "../base/Error.h"
#include "Graph.h"
#include "Log.h"


namespace uquad
{
namespace media
{
	RawWriter::Registry::Registry()
		: Writer::Registry("raw")
	{
	}

	RawWriter::Registry::~Registry()
	{
	}
    
    eContainerFormat RawWriter::Registry::getContainerFormat() const
    {
        return kContainerFormatRAW;
    }

	intrusive_ptr<media::Component> RawWriter::Registry::createComponent() const
	{
		return intrusive_ptr<media::Component>(new RawWriter());
	}
    
    RawWriter::VideoInputPort::VideoInputPort(Writer &w)
        : media::VideoInputPort()
        , m_Writer(w)
    {
    }
    
    RawWriter::VideoInputPort::~VideoInputPort()
    {
    }
    
    intrusive_ptr<Sink> RawWriter::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_Writer);
    }
    
    bool RawWriter::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoEncoded)
            return false;
        
        return true;
    }
    

	RawWriter::RawWriter()
		: Writer()
        , m_VideoInputPort(*this)
        , m_Stream()
        , m_AsyncStream()
        , m_YieldEvent()
        , m_CurrentVideoBufferSize(0)
        , m_CurrentVideoBuffer()
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
	}

	RawWriter::~RawWriter()
	{
	}

	bool RawWriter::isValid() const
	{
		return true;
	}
    
    system::error_code RawWriter::setStream(intrusive_ptr<base::IOStream> const &stm)
    {
        if(isRunning())
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::setStream: can not change stream while is running";
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_Stream = stm;
        m_AsyncStream = dynamic_pointer_cast<base::IOAsyncStream>(m_Stream);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code RawWriter::prepare(asio::yield_context &yctx)
    {
        if(system::error_code pe = Writer::prepare(yctx))
            return pe;
        
        m_YieldEvent.reset(new base::YieldEvent(getGraph()->getRunloop()->ios()));
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawWriter::unprepare(asio::yield_context &yctx)
    {
        m_YieldEvent.reset();
        Writer::unprepare(yctx);
    }
    
    system::error_code RawWriter::resume(asio::yield_context &yctx)
    {
        if(system::error_code we = Writer::resume(yctx))
            return we;
        
        if(m_VideoInputPort.getSampleType() != m_VideoSampleFormat.sampleType)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::resume: videoSampleFormat.type not same as on input port";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(!m_Stream)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::resume: stream not set!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(!m_Stream->isOpen())
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::resume: stream not opened!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawWriter::pause(asio::yield_context &yctx)
    {
        Writer::pause(yctx);
    }
    
    system::error_code RawWriter::run(asio::yield_context &yctx)
    {
        if(system::error_code we = Writer::run(yctx))
            return we;
        
        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::run: could not get videoSample!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(!videoSample.buffer || videoSample.buffer->lock())
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::run: could not get/lock videoSample.buffer!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        m_CurrentVideoBuffer = videoSample.buffer;
        m_CurrentVideoBufferSize = m_CurrentVideoBuffer->size();
        
        system::error_code ewvb;
        if(m_AsyncStream)
        {
            ewvb = m_YieldEvent->waitBlock(yctx, [this]() -> system::error_code
            {
                asyncWriteVideoBuffer();
                return base::makeErrorCode(base::kENoError);
            });
        }
        else
        {
            ewvb = writeVideoBuffer();
        }
        
        if(ewvb)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::run: could not write video buffer!";
        }
        return ewvb;
    }
    
    void RawWriter::rest(asio::yield_context &yctx)
    {
        m_CurrentVideoBuffer->unlock();
        m_CurrentVideoBuffer.reset();
        Writer::rest(yctx);
    }
        
    std::size_t RawWriter::numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<InputPort> RawWriter::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);
        
        return intrusive_ptr<InputPort>();
    }
    
    eContainerFormat RawWriter::getContainerFormat() const
    {
        return kContainerFormatRAW;
    }
    
    VideoSampleFormat const& RawWriter::getVideoSampleFormat() const
    {
        return m_VideoSampleFormat;
    }
    
    system::error_code RawWriter::setVideoSampleFormat(VideoSampleFormat const &fmt)
    {
        if(isRunning())
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::setVideoSampleFormat: can not change video sample format while is running";
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_VideoSampleFormat = fmt;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code RawWriter::writeVideoBuffer()
    {
        std::size_t n = m_Stream->write(&m_CurrentVideoBufferSize, sizeof(uint32_t));
        if(n != sizeof(uint32_t))
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::writeVideoBuffer: size write failed n=" << n;
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        n = m_Stream->write(m_CurrentVideoBuffer->data(), m_CurrentVideoBufferSize);
        if(n != m_CurrentVideoBufferSize)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::writeVideoBuffer: data write failed " << n << "/" << m_CurrentVideoBufferSize;
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawWriter::asyncWriteVideoBuffer()
    {
        m_AsyncStream->asyncWrite(&m_CurrentVideoBufferSize, sizeof(uint32_t), bind(&RawWriter::handleAsyncWriteVideoBufferSize, this, _1, _2));
    }
    
    void RawWriter::handleAsyncWriteVideoBufferSize(system::error_code e, std::size_t n)
    {
        if(e || n != sizeof(uint32_t))
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::handleAsyncWriteVideoBufferSize: write failed n=" << n << ", e=" << e;
            m_YieldEvent->set(base::makeErrorCode(base::kEInvalidState));
        }
        else
        {
            m_AsyncStream->asyncWrite(
                m_CurrentVideoBuffer->data(),
                m_CurrentVideoBufferSize,
                bind(
                    &RawWriter::handleAsyncWriteVideoBufferData,
                    this, _1, _2, 0
                )
            );
        }
    }
    
    void RawWriter::handleAsyncWriteVideoBufferData(system::error_code e, std::size_t n, std::size_t offset)
    {
        if(e || n == 0)
        {
            UQUAD_MEDIA_LOG(Error) << "RawWriter::handleAsyncWriteVideoBufferData: write failed n=" << n << ", e=" << e;
            m_YieldEvent->set(base::makeErrorCode(base::kEInvalidState));
        }
        else
        {
            std::size_t nextOffset = offset + n;
            if(nextOffset < m_CurrentVideoBufferSize)
            {
                m_AsyncStream->asyncWrite(
                    reinterpret_cast<uint8_t*>(m_CurrentVideoBuffer->data())+offset+n,
                    m_CurrentVideoBufferSize - nextOffset,
                    bind(
                        &RawWriter::handleAsyncWriteVideoBufferData,
                        this, _1, _2, nextOffset
                    )
                );
            }
            else
            {
                m_YieldEvent->set(base::makeErrorCode(base::kENoError));
            }
        }
    }
    
} //namespace media
} //namespace uquad
