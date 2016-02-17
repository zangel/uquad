#include "RawReader.h"
#include "../base/Error.h"
#include "Log.h"
#include "Graph.h"

namespace uquad
{
namespace media
{
	RawReader::Registry::Registry()
		: Reader::Registry("raw")
	{
	}

	RawReader::Registry::~Registry()
	{
	}
    
    eContainerFormat RawReader::Registry::getContainerFormat() const
    {
        return kContainerFormatRAW;
    }

	intrusive_ptr<media::Component> RawReader::Registry::createComponent() const
	{
		return intrusive_ptr<media::Component>(new RawReader());
	}
    
    RawReader::VideoOutputPort::VideoOutputPort(RawReader &rrdr)
        : media::VideoOutputPort()
        , m_Reader(rrdr)
    {
    }
    
    RawReader::VideoOutputPort::~VideoOutputPort()
    {
    }
    
    intrusive_ptr<Source> RawReader::VideoOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(const_cast<RawReader*>(&m_Reader));
    }
    
    SampleType RawReader::VideoOutputPort::getSampleType() const
    {
        return m_Reader.getOutputPortSampleType();
    }
    
    Sample RawReader::VideoOutputPort::getSample() const
    {
        return m_Reader.getOutputPortSample();
    }
    
    float RawReader::VideoOutputPort::getSampleRate() const
    {
        return m_Reader.getOutputPortSampleRate();
    }
    
    Vec2i RawReader::VideoOutputPort::getVideoSampleSize() const
    {
        return m_Reader.getOutputPortVideoSampleSize();
    }
    
    RawReader::VideoOutputBuffer::VideoOutputBuffer()
        : m_Data()
        , m_Size(0)
    {
    }
    
    RawReader::VideoOutputBuffer::~VideoOutputBuffer()
    {
    }
    
    bool RawReader::VideoOutputBuffer::isValid() const
    {
        return true;
    }
    
    void* RawReader::VideoOutputBuffer::data()
    {
        return m_Data.data();
    }
    
    std::size_t RawReader::VideoOutputBuffer::size() const
    {
        return m_Size;
    }
    
    
    system::error_code RawReader::VideoOutputBuffer::lock()
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code RawReader::VideoOutputBuffer::unlock()
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    
    RawReader::RawReader()
		: Reader()
        , m_VideoOutputPort(*this)
        , m_VideoOutputBuffer()
        , m_pVideoOutputBuffer()
        , m_Stream()
        , m_AsyncStream()
        , m_VideoSampleTime(0.0f)
        , m_YieldEvent()
    {
        intrusive_ptr_add_ref(&m_VideoOutputPort);
        intrusive_ptr_add_ref(&m_VideoOutputBuffer);
	}

	RawReader::~RawReader()
	{
	}

	bool RawReader::isValid() const
	{
		return true;
	}
    
    system::error_code RawReader::prepare(asio::yield_context &yctx)
    {
        if(system::error_code pe = Reader::prepare(yctx))
            return pe;
        
        m_YieldEvent.reset(new base::YieldEvent(getGraph()->getRunloop()->ios()));
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawReader::unprepare(asio::yield_context &yctx)
    {
        m_Stream->close();
        m_YieldEvent.reset();
        Reader::unprepare(yctx);
    }
    
    system::error_code RawReader::resume(asio::yield_context &yctx)
    {
        if(system::error_code re = Reader::resume(yctx))
            return re;
        
        if(!m_Stream)
        {
            UQUAD_MEDIA_LOG(Error) << "RawReader::resume: stream not set!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(!m_Stream->isOpen())
        {
            UQUAD_MEDIA_LOG(Error) << "RawReader::resume: stream not opened!";
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        m_VideoSampleTime = 0.0f;
        
        if(m_AsyncStream)
        {
            asyncReadVideoBuffer();
        }
        else
        {
            readVideoBuffer();
        }
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawReader::pause(asio::yield_context &yctx)
    {
        Reader::pause(yctx);
    }
    
    system::error_code RawReader::run(asio::yield_context &yctx)
    {
        if(system::error_code re = Reader::run(yctx))
            return re;
        
        m_pVideoOutputBuffer.reset(&m_VideoOutputBuffer);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void RawReader::rest(asio::yield_context &yctx)
    {
        m_pVideoOutputBuffer.reset();
        if(m_AsyncStream)
        {
            asyncReadVideoBuffer();
        }
        else
        {
            readVideoBuffer();
        }
        Reader::rest(yctx);
    }
    
    std::size_t RawReader::numOutputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<OutputPort> RawReader::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_VideoOutputPort);
        
        return intrusive_ptr<OutputPort>();
    }
    
    eContainerFormat RawReader::getContainerFormat() const
    {
        return kContainerFormatRAW;
    }
    
    VideoSampleFormat const& RawReader::getVideoSampleFormat() const
    {
        return m_VideoSampleFormat;
    }
        
    system::error_code RawReader::setVideoSampleFormat(VideoSampleFormat const &fmt)
    {
        if(isRunning())
        {
            UQUAD_MEDIA_LOG(Error) << "RawReader::setVideoSampleFormat: can not change video sample format while is running";
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        m_VideoSampleFormat = fmt;
        return base::makeErrorCode(base::kENoError);
    }
    
    
    system::error_code RawReader::setStream(intrusive_ptr<base::IOStream> const &stm)
    {
        if(isRunning())
            return base::makeErrorCode(base::kEInvalidState);
        
        m_Stream = stm;
        m_AsyncStream = dynamic_pointer_cast<base::IOAsyncStream>(m_Stream);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    SampleType RawReader::getOutputPortSampleType() const
    {
        return m_VideoSampleFormat.sampleType;
    }
    
    Sample RawReader::getOutputPortSample() const
    {
        if(!m_pVideoOutputBuffer)
            return Sample();
        
        return Sample(m_pVideoOutputBuffer, 0.0f);
    }
    
    float RawReader::getOutputPortSampleRate() const
    {
        return m_VideoSampleFormat.sampleRate;
    }
    
    Vec2i RawReader::getOutputPortVideoSampleSize() const
    {
        return m_VideoSampleFormat.sampleSize;
    }
    
    void RawReader::readVideoBuffer()
    {
        if( m_Stream->read(&m_VideoOutputBuffer.m_Size, sizeof(uint32_t)) != sizeof(uint32_t) || !m_VideoOutputBuffer.m_Size)
            return;
        
        m_VideoOutputBuffer.m_Data.resize(m_VideoOutputBuffer.m_Size);
        
        if(m_Stream->read(m_VideoOutputBuffer.m_Data.data(), m_VideoOutputBuffer.m_Size) != m_VideoOutputBuffer.m_Size)
            return;
        
        notifyOnSampleGenerated();
    }
    
    void RawReader::asyncReadVideoBuffer()
    {
        m_AsyncStream->asyncRead(&m_VideoOutputBuffer.m_Size, sizeof(uint32_t), bind(&RawReader::handleAsyncReadVideoBufferSize, this, _1, _2));
    }
    
    void RawReader::handleAsyncReadVideoBufferSize(system::error_code e, std::size_t n)
    {
        if(e || n != sizeof(uint32_t))
        {

        }
        else
        {
            m_VideoOutputBuffer.m_Data.resize(m_VideoOutputBuffer.m_Size);
            m_AsyncStream->asyncRead(
                m_VideoOutputBuffer.m_Data.data(),
                m_VideoOutputBuffer.m_Size,
                bind(
                    &RawReader::handleAsyncReadVideoBufferData,
                    this, _1, _2, 0
                )
            );
        }
    }
    
    void RawReader::handleAsyncReadVideoBufferData(system::error_code e, std::size_t n, std::size_t offset)
    {
        if(e || n == 0)
        {

        }
        else
        {
            std::size_t nextOffset = offset + n;
            if(nextOffset < m_VideoOutputBuffer.m_Size)
            {
                m_AsyncStream->asyncRead(
                    m_VideoOutputBuffer.m_Data.data()+offset+n,
                    m_VideoOutputBuffer.m_Size - nextOffset,
                    bind(
                        &RawReader::handleAsyncReadVideoBufferData,
                        this, _1, _2, nextOffset
                    )
                );
            }
            else
            {
                notifyOnSampleGenerated();
            }
        }
    }
    
} //namespace media
} //namespace uquad
