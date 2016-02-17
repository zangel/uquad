#include "VideoEncoder.h"
#include "../../base/Error.h"
#include "OMXCore.h"
#include "../../math/FixedPoint.h"
#include "../../hal/android/MetaPixelSampleBuffer.h"
#include "../Graph.h"
#include "../Log.h"

namespace uquad
{
namespace media
{
namespace omx
{
    namespace
    {
        template<class T>
        static void initOMXParams(T *params)
        {
            params->nSize = sizeof(T);
            params->nVersion.s.nVersionMajor = 1;
            params->nVersion.s.nVersionMinor = 0;
            params->nVersion.s.nRevision = 0;
            params->nVersion.s.nStep = 0;
        }
    }

    VideoEncoder::Registry::Registry(std::string const &n, eVideoCodecType vct, std::size_t oci)
        : media::VideoEncoder::Registry(n)
        , videoCodecType(vct)
        , ocxIndex(oci)
    {
    }

    VideoEncoder::Registry::~Registry()
    {
    }

    eVideoCodecType VideoEncoder::Registry::getVideoCodecType() const
    {
        return videoCodecType;
    }

    intrusive_ptr<media::Component> VideoEncoder::Registry::createComponent() const
    {
        return intrusive_ptr<media::Component>(new VideoEncoder(*this));
    }

    VideoEncoder::VideoInputPort::VideoInputPort(VideoEncoder &ve)
        : media::VideoInputPort()
        , m_VideoEncoder(ve)
    {
    }

    VideoEncoder::VideoInputPort::~VideoInputPort()
    {
    }

    intrusive_ptr<Sink> VideoEncoder::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_VideoEncoder);
    }

    bool VideoEncoder::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoRaw)
            return false;

        return m_VideoEncoder.acceptsInputPortPixelFormat(outSampleType.kind.videoRaw);

    }

    VideoEncoder::VideoOutputPort::VideoOutputPort(VideoEncoder &ve)
        : media::VideoOutputPort()
        , m_VideoEncoder(ve)
    {
    }

    VideoEncoder::VideoOutputPort::~VideoOutputPort()
    {
    }

    intrusive_ptr<Source> VideoEncoder::VideoOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(&m_VideoEncoder);
    }


    SampleType VideoEncoder::VideoOutputPort::getSampleType() const
    {
        return m_VideoEncoder.getOutputPortSampleType();
    }

    Sample VideoEncoder::VideoOutputPort::getSample() const
    {
        return m_VideoEncoder.getOutputPortSample();
    }

    float VideoEncoder::VideoOutputPort::getSampleRate() const
    {
        return m_VideoEncoder.getOutputPortSampleRate();
    }

    Vec2i VideoEncoder::VideoOutputPort::getVideoSampleSize() const
    {
        return m_VideoEncoder.getOutputPortVideoSampleSize();
    }

    VideoEncoder::VideoOutputBuffer::VideoOutputBuffer(VideoEncoder &ve, OMX_BUFFERHEADERTYPE *bh)
        : m_VideoEncoder(ve)
        , m_BufferHeader(bh)
        , m_LockCounter(0)
    {
    }

    VideoEncoder::VideoOutputBuffer::~VideoOutputBuffer()
    {
        if(m_BufferHeader)
            m_VideoEncoder.returnOMXOutputBuffer(m_BufferHeader);
    }

    bool VideoEncoder::VideoOutputBuffer::isValid() const
    {
        return m_BufferHeader;
    }

    void* VideoEncoder::VideoOutputBuffer::data()
    {
        return m_BufferHeader->pBuffer;
    }

    std::size_t VideoEncoder::VideoOutputBuffer::size() const
    {
        return m_BufferHeader->nFilledLen;
    }

    system::error_code VideoEncoder::VideoOutputBuffer::lock()
    {
        if(!isValid())
            return base::makeErrorCode(base::kEInvalidContent);

        if(m_LockCounter++)
        {
            return base::makeErrorCode(base::kENoError);
        }

        return base::makeErrorCode(base::kENoError);
    }

    system::error_code VideoEncoder::VideoOutputBuffer::unlock()
    {
        if(!isValid())
            return base::makeErrorCode(base::kEInvalidContent);

        if(!m_LockCounter)
        {
            return base::makeErrorCode(base::kENotApplicable);
        }

        if(--m_LockCounter)
        {
            return base::makeErrorCode(base::kENoError);
        }

        return base::makeErrorCode(base::kENoError);
    }

    VideoEncoder::VideoEncoder(Registry const &r)
        : media::VideoEncoder()
        , m_Registry(r)
        , m_OMXHandle(0)
        , m_ExtStoreMetaDataInBuffersIndex(OMX_IndexComponentStartUnused)
        , m_ExtPrependSPSPPSToIDRFramesIndex(OMX_IndexComponentStartUnused)
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_pVideoInputBuffer()
        , m_pVideoOutputBuffer()
        , m_Profile(kCodecProfileLowLatency)
        , m_VideoBitRate(768000)
        , m_VideoSampleSize(0,0)
        , m_VideoSampleRate(0.0f)
        , m_VideoSampleTime(0.0f)
        , m_OMXState(OMX_StateInvalid)
        , m_OMXInputPortBufferSize(0)
        , m_OMXInputPortBuffers(0)
        , m_OMXInputPortBuffersQueue()
        , m_OMXOutputPortBufferSize(0)
        , m_OMXOutputPortBuffers(0)
        , m_OMXOutputPortBuffersQueue()
        , m_EBDYieldEvent()
        , m_StateYieldEvent()

    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);
    }

    VideoEncoder::~VideoEncoder()
    {

    }

    bool VideoEncoder::isValid() const
    {
        return true;
    }

    system::error_code VideoEncoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code ee = media::VideoEncoder::prepare(yctx))
            return ee;

        OMX_CALLBACKTYPE callbacks;
        callbacks.EventHandler = &VideoEncoder::_omxEventHandler;
        callbacks.EmptyBufferDone = &VideoEncoder::_omxEmptyBufferDone;
        callbacks.FillBufferDone = &VideoEncoder::_omxFillBufferDone;
        SI(OMXCore).createComponentHandle(m_OMXHandle, m_Registry.ocxIndex, this, &callbacks);

        if(m_OMXHandle == 0)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not create omx hadle!";
            return base::makeErrorCode(base::kEInvalidState);
        }

        OMX_GetExtensionIndex(m_OMXHandle, const_cast<OMX_STRING>(OMXCore::EXT_OMX_google_android_index_storeMetaDataInBuffers), &m_ExtStoreMetaDataInBuffersIndex);
        OMX_GetExtensionIndex(m_OMXHandle, const_cast<OMX_STRING>(OMXCore::EXT_OMX_google_android_index_prependSPSPPSToIDRFrames), &m_ExtPrependSPSPPSToIDRFramesIndex);

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::prepare: \n"
                << std::showbase << std::internal << std::setfill('0') << std::hex << std::setw(10)
                << "    m_ExtStoreMetaDataInBuffersIndex=" << m_ExtStoreMetaDataInBuffersIndex << "\n"
                << "    m_ExtPrependSPSPPSToIDRFramesIndex=" << m_ExtPrependSPSPPSToIDRFramesIndex;

        if( m_ExtStoreMetaDataInBuffersIndex == OMX_IndexComponentStartUnused ||
            m_ExtPrependSPSPPSToIDRFramesIndex == OMX_IndexComponentStartUnused)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: " << OMXCore::EXT_OMX_google_android_index_storeMetaDataInBuffers << " or " << OMXCore::EXT_OMX_google_android_index_prependSPSPPSToIDRFrames << " is not supported!";
            return base::makeErrorCode(base::kEInvalidState);
        }

        OMX_STATETYPE omxState;
        if(OMX_GetState(m_OMXHandle, &omxState) != OMX_ErrorNone || omxState != OMX_StateLoaded)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: invalid omx state";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        m_OMXState = omxState;

        m_VideoSampleSize = m_VideoInputPort.getVideoSampleSize();
        m_VideoSampleRate = m_VideoInputPort.getSampleRate();
        m_VideoSampleTime = 0.0f;

        //input port
        if(!setupInputPort())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not setup input port!";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        //output port
        if(!setupOutputPort())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not setup output port!";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        //profile
        if(!setupProfileAndLevel())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not setup profile and level!";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        //bitrate
        if(!setupBitrate())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not setup bitrate!";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        //framerate
        if(!setupFramerate())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::prepare: could not setup framerate!";
            SI(OMXCore).destroyComponentHandle(m_OMXHandle);
            m_OMXHandle = 0;
            return base::makeErrorCode(base::kEInvalidState);
        }

        m_EBDYieldEvent.reset(new base::YieldEvent(getGraph()->getRunloop()->ios()));
        m_StateYieldEvent.reset(new base::YieldEvent(getGraph()->getRunloop()->ios()));


        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::prepare: changing state to OMX_StateIdle ...";
        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            omxAllocBuffers();
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StateIdle, 0);
            return base::makeErrorCode(base::kENoError);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::prepare: changing state to OMX_StateExecuting ...";
        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StateExecuting, 0);
            return base::makeErrorCode(base::kENoError);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::prepare: ip=" << m_OMXInputPortBuffersQueue.read_available() << ", op=" << m_OMXOutputPortBuffersQueue.read_available();
        //fill input buffers queue
        std::for_each(m_OMXInputPortBuffers.begin(), m_OMXInputPortBuffers.end(), [this](OMX_BUFFERHEADERTYPE *buffer)
        {
            m_OMXInputPortBuffersQueue.push(buffer);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::prepare: prepare output buffers for filling ...";
        //schedule all output buffers for filling (queue is empty)
        std::for_each(m_OMXOutputPortBuffers.begin(), m_OMXOutputPortBuffers.end(), [this](OMX_BUFFERHEADERTYPE *buffer)
        {
            OMX_FillThisBuffer(m_OMXHandle, buffer);
        });

        return base::makeErrorCode(base::kENoError);
    }

    void VideoEncoder::unprepare(asio::yield_context &yctx)
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::unprepare: before idle ip=" << m_OMXInputPortBuffersQueue.read_available() << ", op=" << m_OMXOutputPortBuffersQueue.read_available();
        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StateIdle, 0);
            return base::makeErrorCode(base::kENoError);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::unprepare: idle: ip=" << m_OMXInputPortBuffersQueue.read_available() << ", op=" << m_OMXOutputPortBuffersQueue.read_available();

        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StateLoaded, 0);
            omxFreeBuffers();
            return base::makeErrorCode(base::kENoError);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::unprepare: loaded: ip=" << m_OMXInputPortBuffersQueue.read_available() << ", op=" << m_OMXOutputPortBuffersQueue.read_available();

        m_OMXInputPortBufferSize = 0;
        m_OMXOutputPortBufferSize = 0;
        m_OMXInputPortBuffersQueue.reset();
        m_OMXOutputPortBuffersQueue.reset();

        if(system::error_code de = SI(OMXCore).destroyComponentHandle(m_OMXHandle))
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::unprepare: could not destroy omx handle: " << de;
        }

        m_OMXHandle = 0;
        m_OMXState = OMX_StateInvalid;

        m_EBDYieldEvent.reset();
        m_StateYieldEvent.reset();

        m_ExtStoreMetaDataInBuffersIndex = OMX_IndexComponentStartUnused;
        m_ExtPrependSPSPPSToIDRFramesIndex = OMX_IndexComponentStartUnused;
        media::VideoEncoder::unprepare(yctx);
    }

    system::error_code VideoEncoder::resume(asio::yield_context &yctx)
    {
        if(system::error_code ee = media::VideoEncoder::resume(yctx))
            return ee;

        /*UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::resume: fill output buffers ...";
        std::for_each(m_OMXOutputPortBuffers.begin(), m_OMXOutputPortBuffers.end(), [this](OMX_BUFFERHEADERTYPE *buffer)
        {
            OMX_FillThisBuffer(m_OMXHandle, buffer);
        });

        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::resume: changing state to OMX_StateExecuting ...";
        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StateExecuting, 0);
            return base::makeErrorCode(base::kENoError);
        });
        */

        //m_LastStatsTime = chrono::steady_clock::now();
        //m_EncodeTimeStats = float_stats_t();
        //m_BandwidthStats = float_stats_t();
        //m_BytesEncoded = 0;

        return base::makeErrorCode(base::kENoError);
    }

    void VideoEncoder::pause(asio::yield_context &yctx)
    {
        /*UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::pause: changing state to OMX_StatePause ...";
        m_StateYieldEvent->waitBlock(yctx, [this]() -> system::error_code
        {
            OMX_SendCommand(m_OMXHandle, OMX_CommandStateSet, OMX_StatePause, 0);
            return base::makeErrorCode(base::kENoError);
        });
        */

        media::VideoEncoder::pause(yctx);
    }

    system::error_code VideoEncoder::run(asio::yield_context &yctx)
    {
        if(system::error_code ee = media::VideoEncoder::run(yctx))
            return ee;

        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: no video sample!";
            return base::makeErrorCode(base::kEInvalidContent);
        }

        intrusive_ptr<hal::android::MetaPixelSampleBuffer> pixelSampleBuffer = dynamic_pointer_cast<hal::android::MetaPixelSampleBuffer>(videoSample.buffer);

        if(!pixelSampleBuffer)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: could not get MetaPixelSampleBuffer!";
            return base::makeErrorCode(base::kEInvalidContent);
        }

        if(m_OMXInputPortBuffers.empty())
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: no input buffers available!";
            return base::makeErrorCode(base::kEInvalidContent);
        }

        OMX_BUFFERHEADERTYPE *pInputBuffer;
        if(!m_OMXInputPortBuffersQueue.pop(pInputBuffer))
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: could not get input buffer";
            return base::makeErrorCode(base::kEInvalidContent);
        }

        //fill in meta buffer info
       *(reinterpret_cast<::android::encoder_media_buffer_type *>(pInputBuffer->pBuffer)) = *(pixelSampleBuffer->encoderMediaBuffer());
        pInputBuffer->nFilledLen = sizeof(::android::encoder_media_buffer_type);
        pInputBuffer->nOffset = 0;


        //empty the buffer
        //chrono::steady_clock::time_point start_encode = chrono::steady_clock::now();
        system::error_code err = m_EBDYieldEvent->waitBlockFor(yctx, std::chrono::milliseconds(300), [this, pInputBuffer]() -> system::error_code
        {
            OMX_EmptyThisBuffer(m_OMXHandle, pInputBuffer);
            return base::makeErrorCode(base::kENoError);
        });

        if(err)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: empty buffer error:" << err;
            return err;
        }

        //m_EncodeTimeStats(chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - start_encode).count());

        OMX_BUFFERHEADERTYPE *pOutputBuffer;
        if(!m_OMXOutputPortBuffersQueue.pop(pOutputBuffer))
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::run: no output buffer filled!";
            return base::makeErrorCode(base::kEInvalidContent);
        }

        m_pVideoOutputBuffer.reset(new VideoOutputBuffer(*this, pOutputBuffer));
        //m_BytesEncoded += m_pVideoOutputBuffer->size();

        return base::makeErrorCode(base::kENoError);
    }

    void VideoEncoder::rest(asio::yield_context &yctx)
    {
        /*
        chrono::steady_clock::duration statsElapsed = chrono::steady_clock::now() - m_LastStatsTime;
        if(statsElapsed > chrono::seconds(1))
        {
            m_LastStatsTime = chrono::steady_clock::now();
            m_BandwidthStats(static_cast<float>(m_BytesEncoded)/chrono::duration_cast<chrono::milliseconds>(statsElapsed).count());
            m_BytesEncoded = 0;
            UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::rest: stats:\n"
                    << "    encode(" << accumulators::min(m_EncodeTimeStats) << "/" << accumulators::mean(m_EncodeTimeStats) << "/" << accumulators::max(m_EncodeTimeStats) << ")\n"
                    << "    bandwidth(" << accumulators::min(m_BandwidthStats) << "/" << accumulators::mean(m_BandwidthStats) << "/" << accumulators::max(m_BandwidthStats) << ")";
        }
        */

        m_pVideoInputBuffer.reset();
        m_pVideoOutputBuffer.reset();
        media::VideoEncoder::rest(yctx);
    }

    std::size_t VideoEncoder::numInputPorts() const
    {
        return 1;
    }

    intrusive_ptr<InputPort> VideoEncoder::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);

        return intrusive_ptr<InputPort>();
    }

    std::size_t VideoEncoder::numOutputPorts() const
    {
        return 1;
    }

    intrusive_ptr<OutputPort> VideoEncoder::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_VideoOutputPort);

        return intrusive_ptr<OutputPort>();
    }

    eCodecProfile VideoEncoder::getProfile() const
    {
        return m_Profile;
    }

    bool VideoEncoder::setProfile(eCodecProfile profile)
    {
        if(isRunning())
            return false;
        if(!(profile > 0 && profile < kCodecProfileMax))
            return false;
        m_Profile = profile;
        return true;
    }

    int VideoEncoder::getBitRate() const
    {
        return m_VideoBitRate;
    }

    bool VideoEncoder::setBitRate(int br)
    {
        if(isRunning())
            return false;

        m_VideoBitRate = br;
        return true;
    }

    eVideoCodecType VideoEncoder::getVideoCodecType() const
    {
        return m_Registry.videoCodecType;
    }

    SampleType VideoEncoder::getOutputPortSampleType() const
    {
        return SampleType(getVideoCodecType());
    }

    Sample VideoEncoder::getOutputPortSample() const
    {
        return Sample(m_pVideoOutputBuffer, m_VideoSampleTime);
    }

    float VideoEncoder::getOutputPortSampleRate() const
    {
        return m_VideoSampleRate;
    }

    Vec2i VideoEncoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoSampleSize;
    }

    bool VideoEncoder::acceptsInputPortPixelFormat(base::ePixelFormat fmt) const
    {
        return fmt == base::kPixelFormatYUV420sp;
    }

    bool VideoEncoder::setupInputPort()
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupInputPort: setting up input port ...";
        OMX_PARAM_PORTDEFINITIONTYPE portDef;
        initOMXParams(&portDef);
        portDef.nPortIndex = 0;

        if(OMX_GetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef) != OMX_ErrorNone)
            return false;

        //portdef.nBufferCountActual = portdef.nBufferCountMin;
        portDef.format.video.nFrameWidth = m_VideoSampleSize(0);
        portDef.format.video.nFrameHeight = m_VideoSampleSize(1);
        portDef.format.video.xFramerate = math::fixp32_t(m_VideoInputPort.getSampleRate()).get();
        portDef.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
        portDef.format.video.eColorFormat = static_cast<OMX_COLOR_FORMATTYPE>(QOMX_COLOR_FORMATYUV420PackedSemiPlanar32m);

        if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef) != OMX_ErrorNone)
            return false;

        ::android::StoreMetaDataInBuffersParams enableStoreMetaData;
        initOMXParams(&enableStoreMetaData);
        enableStoreMetaData.nPortIndex = 0;
        enableStoreMetaData.bStoreMetaData = OMX_TRUE;
        if(OMX_SetParameter(m_OMXHandle, m_ExtStoreMetaDataInBuffersIndex, &enableStoreMetaData) != OMX_ErrorNone)
            return false;

        OMX_GetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef);

        m_OMXInputPortBufferSize = portDef.nBufferSize;
        m_OMXInputPortBuffers.resize(portDef.nBufferCountActual, 0);
        return true;
    }

    bool VideoEncoder::setupOutputPort()
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupOutputPort: setting up output port ...";
        OMX_PARAM_PORTDEFINITIONTYPE portDef;
        initOMXParams(&portDef);
        portDef.nPortIndex = 1;

        if(OMX_GetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef) != OMX_ErrorNone)
            return false;

        //portdef.nBufferCountActual = portdef.nBufferCountMin;
        portDef.format.video.nFrameWidth = m_VideoSampleSize(0);
        portDef.format.video.nFrameHeight = m_VideoSampleSize(1);
        portDef.format.video.nBitrate = m_VideoBitRate;
        //portdef.format.video.xFramerate = math::fixp32_t(m_VideoInputPort.getSampleRate()).get();

        portDef.format.video.eCompressionFormat = OMXCore::omxCodingTypeFromVideoCodecType(m_Registry.videoCodecType);
        portDef.format.video.eColorFormat = OMX_COLOR_FormatUnused;

        if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef) != OMX_ErrorNone)
            return false;

        ::android::PrependSPSPPSToIDRFramesParams prependSPSPPS;
        initOMXParams(&prependSPSPPS);
        prependSPSPPS.bEnable = OMX_TRUE;
        if(OMX_SetParameter(m_OMXHandle, m_ExtPrependSPSPPSToIDRFramesIndex, &prependSPSPPS) != OMX_ErrorNone)
            return false;

        OMX_GetParameter(m_OMXHandle, OMX_IndexParamPortDefinition, &portDef);

        m_OMXOutputPortBufferSize = portDef.nBufferSize;
        m_OMXOutputPortBuffers.resize(portDef.nBufferCountActual, 0);
        return true;
    }

    bool VideoEncoder::setupProfileAndLevel()
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupProfileAndLevel: setting up profile and level ...";
        ProfileLevel const *pProfileLevel = OMXCore::chooseOMXProfileLevel(m_Registry.videoCodecType, m_Profile, m_VideoSampleSize, m_VideoInputPort.getSampleRate(), m_VideoBitRate);
        if(!pProfileLevel)
        {
            return false;
        }



        if(m_Registry.videoCodecType == kVideoCodecTypeMPEG4)
        {
        }
        else if(m_Registry.videoCodecType == kVideoCodecTypeH263)
        {
        }
        else
        if(m_Registry.videoCodecType == kVideoCodecTypeH264)
        {
            UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupProfileAndLevel: (profile, level) = ("
                    << OMXCore::h264ProfileName(static_cast<OMX_VIDEO_AVCPROFILETYPE>(pProfileLevel->profile)) << ", "
                    << OMXCore::h264LevelName(static_cast<OMX_VIDEO_AVCLEVELTYPE>(pProfileLevel->level)) << ")";

            //OMX_VIDEO_AVCLEVELTYPE level = OMX_VIDEO_AVCLevel13;
            OMX_VIDEO_PARAM_AVCTYPE avcParams;
            initOMXParams(&avcParams);
            avcParams.nPortIndex = 1;

            if(OMX_GetParameter(m_OMXHandle, OMX_IndexParamVideoAvc, &avcParams) != OMX_ErrorNone)
                return false;

            avcParams.eProfile = static_cast<OMX_VIDEO_AVCPROFILETYPE>(pProfileLevel->profile);
            avcParams.eLevel = static_cast<OMX_VIDEO_AVCLEVELTYPE>(pProfileLevel->level);

            avcParams.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;

            if(avcParams.eProfile == OMX_VIDEO_AVCProfileBaseline)
            {
                avcParams.nSliceHeaderSpacing = 0;
                avcParams.bUseHadamard = OMX_TRUE;
                avcParams.nRefFrames = 1;
                avcParams.nBFrames = 0;
                avcParams.nPFrames = (static_cast<int>(m_VideoInputPort.getSampleRate()) - 1);
                if(avcParams.nPFrames == 0)
                {
                    avcParams.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI;
                }
                avcParams.nRefIdx10ActiveMinus1 = 0;
                avcParams.nRefIdx11ActiveMinus1 = 0;
                avcParams.bEntropyCodingCABAC = OMX_FALSE;
                avcParams.bWeightedPPrediction = OMX_FALSE;
                avcParams.bconstIpred = OMX_FALSE;
                avcParams.bDirect8x8Inference = OMX_FALSE;
                avcParams.bDirectSpatialTemporal = OMX_FALSE;
                avcParams.nCabacInitIdc = 0;
            }

            if(avcParams.nBFrames != 0)
            {
                avcParams.nAllowedPictureTypes |= OMX_VIDEO_PictureTypeB;
            }

            avcParams.bEnableUEP = OMX_FALSE;
            avcParams.bEnableFMO = OMX_FALSE;
            avcParams.bEnableASO = OMX_FALSE;
            avcParams.bEnableRS = OMX_FALSE;
            avcParams.bFrameMBsOnly = OMX_TRUE;
            avcParams.bMBAFF = OMX_FALSE;
            avcParams.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;

            //param.nBFrames = 0;
            //param.nPFrames = (2 * static_cast<int>(m_VideoInputPort.getSampleRate()) - 1);
            //avcParams.nAllowedPictureTypes = OMX_VIDEO_PictureTypeI | OMX_VIDEO_PictureTypeP;
            //avcParams.eLoopFilterMode = OMX_VIDEO_AVCLoopFilterEnable;
            //avcParams.bEntropyCodingCABAC = OMX_FALSE;
            //avcParams.nCabacInitIdc = 1;

            if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamVideoAvc, &avcParams) != OMX_ErrorNone)
                return false;

            OMX_VIDEO_PARAM_PROFILELEVELTYPE profileLevelParams;
            initOMXParams(&profileLevelParams);
            profileLevelParams.nPortIndex = 1;
            profileLevelParams.eProfile = pProfileLevel->profile;
            profileLevelParams.eLevel =  pProfileLevel->level;
            if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamVideoProfileLevelCurrent, &profileLevelParams) != OMX_ErrorNone)
               return false;
        }
        return true;
    }

    bool VideoEncoder::setupBitrate()
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupBitrate: setting up bitrate ...";
        OMX_VIDEO_PARAM_BITRATETYPE bitrateParams;
        initOMXParams(&bitrateParams);
        bitrateParams.nPortIndex = 1;
        if(OMX_GetParameter(m_OMXHandle, OMX_IndexParamVideoBitrate, &bitrateParams) != OMX_ErrorNone)
            return false;

        bitrateParams.eControlRate = OMX_Video_ControlRateVariable;
        bitrateParams.nTargetBitrate = m_VideoBitRate;
        if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamVideoBitrate, &bitrateParams) != OMX_ErrorNone)
            return false;

        return true;
    }

    bool VideoEncoder::setupFramerate()
    {
        UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::setupFramerate: setting up frame rate ...";
        {
            OMX_VIDEO_PARAM_PORTFORMATTYPE framerate;
            initOMXParams(&framerate);
            framerate.nPortIndex = 0;
            framerate.nIndex = 0;

            if(OMX_GetParameter(m_OMXHandle, OMX_IndexParamVideoPortFormat, &framerate) != OMX_ErrorNone)
                return false;

            framerate.xFramerate = math::fixp32_t(m_VideoInputPort.getSampleRate()).get();
            if(OMX_SetParameter(m_OMXHandle, OMX_IndexParamVideoPortFormat, &framerate) != OMX_ErrorNone)
                return false;
        }

        {
            OMX_CONFIG_FRAMERATETYPE encodingFramerate;
            initOMXParams(&encodingFramerate);
            encodingFramerate.nPortIndex = 1;
            if(OMX_GetConfig(m_OMXHandle, OMX_IndexConfigVideoFramerate, &encodingFramerate) != OMX_ErrorNone)
                return false;

            encodingFramerate.xEncodeFramerate = math::fixp32_t(m_VideoInputPort.getSampleRate()).get();
            if(OMX_SetConfig(m_OMXHandle, OMX_IndexConfigVideoFramerate, &encodingFramerate) != OMX_ErrorNone)
                return false;
        }
        return true;
    }

    OMX_ERRORTYPE VideoEncoder::_omxEventHandler(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd)
    {
        return reinterpret_cast<VideoEncoder*>(ve)->omxEventHandler(evt, nd1, nd2, evtd);
    }

    OMX_ERRORTYPE VideoEncoder::_omxEmptyBufferDone(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_BUFFERHEADERTYPE *buffer)
    {
        return reinterpret_cast<VideoEncoder*>(ve)->omxEmptyBufferDone(buffer);
    }

    OMX_ERRORTYPE VideoEncoder::_omxFillBufferDone(OMX_HANDLETYPE hc, OMX_PTR ve, OMX_BUFFERHEADERTYPE* buffer)
    {
        return reinterpret_cast<VideoEncoder*>(ve)->omxFillBufferDone(buffer);
    }

    OMX_ERRORTYPE VideoEncoder::omxEventHandler(OMX_EVENTTYPE evt, OMX_U32 nd1, OMX_U32 nd2, OMX_PTR evtd)
    {
        if (evt == OMX_EventCmdComplete)
        {
            if((OMX_COMMANDTYPE) nd1 == OMX_CommandStateSet)
            {
                UQUAD_MEDIA_LOG(Debug) << "VideoEncoder::omxEventHandler: state changed: " << nd2;
                m_OMXState = (OMX_STATETYPE) nd2;
                m_StateYieldEvent->set();
            }
        }
        else
        if(evt == OMX_EventError)
        {
            UQUAD_MEDIA_LOG(Error) << "VideoEncoder::omxEventHandler: omxError: (" << nd1 << ", " << nd2 << ")";
        }
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE VideoEncoder::omxEmptyBufferDone(OMX_BUFFERHEADERTYPE *buffer)
    {
        m_OMXInputPortBuffersQueue.push(buffer);
        m_EBDYieldEvent->set();
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE VideoEncoder::omxFillBufferDone(OMX_BUFFERHEADERTYPE* buffer)
    {
        m_OMXOutputPortBuffersQueue.push(buffer);
        return OMX_ErrorNone;
    }

    void VideoEncoder::omxAllocBuffers()
    {
        for(std::size_t ib = 0; ib < m_OMXInputPortBuffers.size(); ++ib)
            OMX_AllocateBuffer(m_OMXHandle, &m_OMXInputPortBuffers[ib], 0, this, m_OMXInputPortBufferSize);

        for(std::size_t ob = 0; ob < m_OMXOutputPortBuffers.size(); ++ob)
            OMX_AllocateBuffer(m_OMXHandle, &m_OMXOutputPortBuffers[ob], 1, this, m_OMXOutputPortBufferSize);
    }

    void VideoEncoder::omxFreeBuffers()
    {
        for(std::size_t ib = 0; ib < m_OMXInputPortBuffers.size(); ++ib)
            OMX_FreeBuffer(m_OMXHandle, 0, m_OMXInputPortBuffers[ib]);
        m_OMXInputPortBuffers.clear();

        for(std::size_t ob = 0; ob < m_OMXOutputPortBuffers.size(); ++ob)
            OMX_FreeBuffer(m_OMXHandle, 1, m_OMXOutputPortBuffers[ob]);
        m_OMXOutputPortBuffers.clear();
    }

    void VideoEncoder::returnOMXOutputBuffer(OMX_BUFFERHEADERTYPE *bh)
    {
        if(isRunning())
        {
            OMX_FillThisBuffer(m_OMXHandle, bh);
        }
    }

} //namespace omx
} //namespace media
} //namespace uquad
