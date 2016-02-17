#include "VideoDecoder.h"
#include "../../base/Error.h"
#include "OMXCore.h"

namespace uquad
{
namespace media
{
namespace omx
{
    VideoDecoder::Registry::Registry(std::string const &n, eVideoCodecType vct, std::size_t oci)
        : media::VideoDecoder::Registry(n)
        , videoCodecType(vct)
        , ocxIndex(oci)
    {
    }

    VideoDecoder::Registry::~Registry()
    {
    }

    eVideoCodecType VideoDecoder::Registry::getVideoCodecType() const
    {
        return videoCodecType;
    }

    intrusive_ptr<media::Component> VideoDecoder::Registry::createComponent() const
    {
        return intrusive_ptr<media::Component>(new VideoDecoder(*this));
    }

    VideoDecoder::VideoInputPort::VideoInputPort(VideoDecoder &vd)
        : media::VideoInputPort()
        , m_VideoDecoder(vd)
    {
    }

    VideoDecoder::VideoInputPort::~VideoInputPort()
    {
    }

    intrusive_ptr<Sink> VideoDecoder::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_VideoDecoder);
    }

    bool VideoDecoder::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoEncoded)
            return false;

        return m_VideoDecoder.acceptsInputPortVideoCodecType(outSampleType.kind.videoEncoded);

    }

    VideoDecoder::VideoOutputPort::VideoOutputPort(VideoDecoder &vd)
        : media::VideoOutputPort()
        , m_VideoDecoder(vd)
    {
    }

    VideoDecoder::VideoOutputPort::~VideoOutputPort()
    {
    }

    intrusive_ptr<Source> VideoDecoder::VideoOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(&m_VideoDecoder);
    }


    SampleType VideoDecoder::VideoOutputPort::getSampleType() const
    {
        return m_VideoDecoder.getOutputPortSampleType();
    }

    Sample VideoDecoder::VideoOutputPort::getSample() const
    {
        return m_VideoDecoder.getOutputPortSample();
    }

    float VideoDecoder::VideoOutputPort::getSampleRate() const
    {
        return m_VideoDecoder.getOutputPortSampleRate();
    }

    Vec2i VideoDecoder::VideoOutputPort::getVideoSampleSize() const
    {
        return m_VideoDecoder.getOutputPortVideoSampleSize();
    }

       VideoDecoder::VideoDecoder(Registry const &r)
        : media::VideoDecoder()
        , m_Registry(r)
        , m_OMXHandle(0)
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_Profile(kCodecProfileLowLatency)
        , m_VideoBitRate(400000)
        , m_VideoSampleSize(0,0)
        , m_VideoSampleRate(0.0f)
        , m_VideoSampleTime(0.0f)
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);

        OMX_CALLBACKTYPE callbacks;
        callbacks.EventHandler = &VideoDecoder::_omxEventHandler;
        callbacks.EmptyBufferDone = &VideoDecoder::_omxEmptyBufferDone;
        callbacks.FillBufferDone = &VideoDecoder::_omxFillBufferDone;
        SI(OMXCore).createComponentHandle(m_OMXHandle, m_Registry.ocxIndex, this, &callbacks);
    }

    VideoDecoder::~VideoDecoder()
    {
        SI(OMXCore).destroyComponentHandle(m_OMXHandle);
    }

    bool VideoDecoder::isValid() const
    {
        return m_OMXHandle != 0;
    }

    system::error_code VideoDecoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code de = media::VideoDecoder::prepare(yctx))
            return de;

        return base::makeErrorCode(base::kEInvalidState);
    }

    void VideoDecoder::unprepare(asio::yield_context &yctx)
    {
        media::VideoDecoder::unprepare(yctx);
    }

    system::error_code VideoDecoder::resume(asio::yield_context &yctx)
    {
        if(system::error_code de = media::VideoDecoder::resume(yctx))
            return de;

        return base::makeErrorCode(base::kEInvalidState);
    }

    void VideoDecoder::pause(asio::yield_context &yctx)
    {
        media::VideoDecoder::pause(yctx);
    }

    system::error_code VideoDecoder::run(asio::yield_context &yctx)
    {
        if(system::error_code de = media::VideoDecoder::run(yctx))
            return de;

        return base::makeErrorCode(base::kEInvalidState);
    }

    void VideoDecoder::rest(asio::yield_context &yctx)
    {
        media::VideoDecoder::rest(yctx);
    }

    std::size_t VideoDecoder::numInputPorts() const
    {
        return 1;
    }

    intrusive_ptr<InputPort> VideoDecoder::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);

        return intrusive_ptr<InputPort>();
    }

    std::size_t VideoDecoder::numOutputPorts() const
    {
        return 1;
    }

    intrusive_ptr<OutputPort> VideoDecoder::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_VideoOutputPort);

        return intrusive_ptr<OutputPort>();
    }

    eCodecProfile VideoDecoder::getProfile() const
    {
        return m_Profile;
    }

    bool VideoDecoder::setProfile(eCodecProfile profile)
    {
        if(isRunning())
            return false;

        if(!(profile > 0 && profile < kCodecProfileMax))
            return false;

        m_Profile = profile;
        return true;
    }

    int VideoDecoder::getBitRate() const
    {
        return m_VideoBitRate;
    }

    bool VideoDecoder::setBitRate(int br)
    {
        if(isRunning())
            return false;

        m_VideoBitRate = br;
        return true;
    }

    eVideoCodecType VideoDecoder::getVideoCodecType() const
    {
        return m_Registry.videoCodecType;
    }

    bool VideoDecoder::acceptsInputPortVideoCodecType(eVideoCodecType vct) const
    {
        if(!vct)
            return false;

        return vct == getVideoCodecType();
    }

    SampleType VideoDecoder::getOutputPortSampleType() const
    {
        return SampleType(base::kPixelFormatYUV420sp);
    }

    Sample VideoDecoder::getOutputPortSample() const
    {
        return Sample();
    }

    float VideoDecoder::getOutputPortSampleRate() const
    {
        return m_VideoSampleRate;
    }

    Vec2i VideoDecoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoSampleSize;
    }

    OMX_ERRORTYPE VideoDecoder::_omxEventHandler(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd)
    {
        return reinterpret_cast<VideoDecoder*>(vd)->omxEventHandler(evt, nd1, nd2, evtd);
    }

    OMX_ERRORTYPE VideoDecoder::_omxEmptyBufferDone(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_BUFFERHEADERTYPE *buffer)
    {
        return reinterpret_cast<VideoDecoder*>(vd)->omxEmptyBufferDone(buffer);
    }

    OMX_ERRORTYPE VideoDecoder::_omxFillBufferDone(OMX_HANDLETYPE hc, OMX_PTR vd, OMX_BUFFERHEADERTYPE* buffer)
    {
        return reinterpret_cast<VideoDecoder*>(vd)->omxFillBufferDone(buffer);
    }

    OMX_ERRORTYPE VideoDecoder::omxEventHandler(OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd)
    {
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE VideoDecoder::omxEmptyBufferDone(OMX_BUFFERHEADERTYPE *buffer)
    {
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE VideoDecoder::omxFillBufferDone(OMX_BUFFERHEADERTYPE* buffer)
    {
        return OMX_ErrorNone;
    }

} //namespace omx
} //namespace media
} //namespace uquad
