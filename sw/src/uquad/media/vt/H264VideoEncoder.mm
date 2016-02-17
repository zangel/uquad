#include "H264VideoEncoder.h"
#include "VT.h"
#include "../../hal/av/CVImagePixelSampleBuffer.h"
#include "../../base/Error.h"
#include <CoreMedia/CoreMedia.h>



namespace uquad
{
namespace media
{
namespace vt
{
    namespace
    {
        template <typename T>
        std::size_t readNALSize(uint8_t const *src);
        
        template <>
        std::size_t readNALSize<uint8_t>(uint8_t const *src)
        {
            return *src;
        }
        
        template <>
        std::size_t readNALSize<uint16_t>(uint8_t const *src)
        {
            return (uint16_t(src[0]) << 8) | src[1];
        }
        
        template <>
        std::size_t readNALSize<uint32_t>(uint8_t const *src)
        {
            return (uint32_t(src[0]) << 24) | (uint32_t(src[1]) << 16) | (uint16_t(src[2]) << 8) | src[3];
        }
        
        
        template <typename T>
        static void copyNALUnits(uint8_t *dst, uint8_t const *src, std::size_t n)
        {
            static const char startCode[3] = {0, 0, 1};
            std::size_t bytesLeft = n;
            while(bytesLeft > 0)
            {
                std::size_t nalSize = readNALSize<T>(src);
                bytesLeft -= sizeof(T);
                src += sizeof(T);
                ::memcpy(dst, startCode, sizeof(startCode));
                ::memcpy(dst + sizeof(startCode), src, nalSize);
                bytesLeft -= nalSize;
                src += nalSize;
                dst += (nalSize + sizeof(startCode));
            }
        }
    }
    
    H264VideoEncoder::Registry::Registry(std::string const &n, CFDictionaryRef es)
        : media::VideoEncoder::Registry(n)
        , encoderSpec(es)
    {
        CFRetain(encoderSpec);
    }
    
    H264VideoEncoder::Registry::~Registry()
    {
        CFRelease(encoderSpec);
    }
    
    eVideoCodecType H264VideoEncoder::Registry::getVideoCodecType() const
    {
        return kVideoCodecTypeH264;
    }
    
	intrusive_ptr<media::Component> H264VideoEncoder::Registry::createComponent() const
    {
        return intrusive_ptr<media::Component>(new H264VideoEncoder(*this));
    }
    
    H264VideoEncoder::VideoInputPort::VideoInputPort(H264VideoEncoder &ve)
        : media::VideoInputPort()
        , m_VideoEncoder(ve)
    {
    }
    
    H264VideoEncoder::VideoInputPort::~VideoInputPort()
    {
    }
    
    intrusive_ptr<Sink> H264VideoEncoder::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_VideoEncoder);
    }
    
    bool H264VideoEncoder::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoRaw)
            return false;
        
        return m_VideoEncoder.acceptsInputPortPixelFormat(outSampleType.kind.videoRaw);
        
    }
    
    H264VideoEncoder::VideoOutputPort::VideoOutputPort(H264VideoEncoder &ve)
        : media::VideoOutputPort()
        , m_VideoEncoder(ve)
    {
    }
    
    H264VideoEncoder::VideoOutputPort::~VideoOutputPort()
    {
    }
    
    intrusive_ptr<Source> H264VideoEncoder::VideoOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(&m_VideoEncoder);
    }
    
    
    SampleType H264VideoEncoder::VideoOutputPort::getSampleType() const
    {
        return m_VideoEncoder.getOutputPortSampleType();
    }
    
    Sample H264VideoEncoder::VideoOutputPort::getSample() const
    {
        return m_VideoEncoder.getOutputPortSample();
    }
    
    float H264VideoEncoder::VideoOutputPort::getSampleRate() const
    {
        return m_VideoEncoder.getOutputPortSampleRate();
    }
    
    Vec2i H264VideoEncoder::VideoOutputPort::getVideoSampleSize() const
    {
        return m_VideoEncoder.getOutputPortVideoSampleSize();
    }
    
    H264VideoEncoder::VideoOutputBuffer::VideoOutputBuffer(CMSampleBufferRef cmsb)
        : m_CMSampleBuffer(cmsb)
        , m_Data()
    {
        CFRetain(m_CMSampleBuffer);
        init();
    }
    
    H264VideoEncoder::VideoOutputBuffer::~VideoOutputBuffer()
    {
        CFRelease(m_CMSampleBuffer);
    }
    
    bool H264VideoEncoder::VideoOutputBuffer::isValid() const
    {
        return m_CMSampleBuffer != 0;
    }
    
    void* H264VideoEncoder::VideoOutputBuffer::data()
    {
        return m_Data.data();
    }
    
    std::size_t H264VideoEncoder::VideoOutputBuffer::size() const
    {
        return m_Data.size();
    }
    
    system::error_code H264VideoEncoder::VideoOutputBuffer::lock()
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code H264VideoEncoder::VideoOutputBuffer::unlock()
    {
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoEncoder::VideoOutputBuffer::init()
    {
        auto sampleAttachments = reinterpret_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(CMSampleBufferGetSampleAttachmentsArray(m_CMSampleBuffer, true), 0));
        bool keyframe = !CFDictionaryContainsKey(sampleAttachments, kCMSampleAttachmentKey_NotSync);
        
        auto dataBuffer = CMSampleBufferGetDataBuffer(m_CMSampleBuffer);
        auto fmtDesc = CMSampleBufferGetFormatDescription(m_CMSampleBuffer);
        std::size_t dataBufferSize = CMBlockBufferGetDataLength(dataBuffer);
        std::size_t totalBytes = dataBufferSize;
        
        std::size_t psetCount;
        int nalSizeFieldBytes;
        
        OSStatus status = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc, 0, nullptr, nullptr, &psetCount, &nalSizeFieldBytes);
        
        if(status == kCMFormatDescriptionBridgeError_InvalidParameter)
        {
            psetCount = 2;
            nalSizeFieldBytes = 4;
        }
        else
        if(status != noErr)
        {
            return;
        }
        
        if(keyframe)
        {
            const uint8_t* pset;
            std::size_t psetSize;
            for(std::size_t ipset = 0; ipset < psetCount; ++ipset)
            {
                status = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc, ipset, &pset, &psetSize, nullptr, nullptr);
                if(status != noErr)
                {
                    return;
                }
                totalBytes += psetSize + nalSizeFieldBytes;
            }
        }
        
        m_Data.resize(totalBytes);
        
        uint8_t *dst = m_Data.data();
        if(keyframe)
        {
            const uint8_t* pset;
            std::size_t psetSize;
            for(std::size_t ipset = 0; ipset < psetCount; ++ipset)
            {
                status = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc, ipset, &pset, &psetSize, nullptr, nullptr);
                if(status != noErr)
                {
                    return;
                }
                
                static const char naluStartCode[4] = {0, 0, 0, 1};
                ::memcpy(dst, naluStartCode, sizeof(naluStartCode));
                ::memcpy(dst + sizeof(naluStartCode), pset, psetSize);
                dst += (sizeof(naluStartCode) + psetSize);
            }
        }
        
        CMBlockBufferRef contiguousDataBuffer = dataBuffer;
        CFRetain(contiguousDataBuffer);
        if(!CMBlockBufferIsRangeContiguous(dataBuffer, 0, 0))
        {
            CFRelease(contiguousDataBuffer);
            status = CMBlockBufferCreateContiguous(kCFAllocatorDefault, dataBuffer, kCFAllocatorDefault, nullptr, 0, 0, 0, &contiguousDataBuffer);
            if(status != noErr)
                return;
        }
        
        uint8_t* pBufferData;
        status = CMBlockBufferGetDataPointer(contiguousDataBuffer, 0, nullptr, nullptr, reinterpret_cast<char**>(&pBufferData));
        if(status != noErr)
            return;
        
        if(nalSizeFieldBytes == 1)
        {
            copyNALUnits<uint8_t>(dst, pBufferData, dataBufferSize);
        }
        else
        if(nalSizeFieldBytes == 2)
        {
            copyNALUnits<uint16_t>(dst, pBufferData, dataBufferSize);
        }
        else
        if(nalSizeFieldBytes == 4)
        {
            copyNALUnits<uint32_t>(dst, pBufferData, dataBufferSize);
        }
        
        CFRelease(contiguousDataBuffer);
    }
    
    
    H264VideoEncoder::H264VideoEncoder(Registry const &r)
		: media::VideoEncoder()
        , m_Registry(r)
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_pVideoOutputBuffer()
        , m_CompressionSession(0)
        , m_ForceVideoKeyFrame(false)
        , m_VideoSampleSize(0,0)
        , m_VideoSampleRate(0.0f)
        , m_VideoSampleTime(0.0f)
        , m_Profile(kCodecProfileLowLatency)
        , m_VideoBitRate(400000)
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);
	}
    
    H264VideoEncoder::~H264VideoEncoder()
	{
	}

	bool H264VideoEncoder::isValid() const
	{
		return media::VideoEncoder::isValid();
	}
    
    system::error_code H264VideoEncoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code e = media::VideoEncoder::prepare(yctx))
            return e;
        
        m_VideoSampleSize = m_VideoInputPort.getVideoSampleSize();
        m_VideoSampleRate = m_VideoInputPort.getSampleRate();
        m_VideoSampleTime = 0.0f;
        
        if(VTCompressionSessionCreate(
            0,
            m_VideoSampleSize[0],
            m_VideoSampleSize[1],
            #if defined(UQUAD_PLATFORM_iOS)
            kCMVideoCodecType_H264,
            0,
            #else
            kCMVideoCodecType_H264,
            m_Registry.encoderSpec,
            #endif
            0,
            0,
            &H264VideoEncoder::_vtCompressionOutputCallback,
            this,
            &m_CompressionSession
        ) != noErr) return base::makeErrorCode(base::kEInvalidState);
        
        ProfileLevel const *profileLevel = VT::chooseVTProfileLevel(kVideoCodecTypeH264, m_Profile, m_VideoSampleSize, m_VideoSampleRate, m_VideoBitRate);
        VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_ProfileLevel, profileLevel->profileLevel);
        
        {
            CFNumberRef bitRate = CFNumberCreate(NULL, kCFNumberIntType, &m_VideoBitRate);
            VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_AverageBitRate, bitRate);
            CFRelease(bitRate);
        }
        {
            CFNumberRef frameRate = CFNumberCreate(NULL, kCFNumberFloatType, &m_VideoSampleRate);
            VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_ExpectedFrameRate, frameRate);
            CFRelease(frameRate);
        }
        
        VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_AllowFrameReordering, kCFBooleanFalse);
        
        #if 0
        {
            SInt32 param = 0;
            CFNumberRef delayCount = CFNumberCreate(NULL, kCFNumberSInt32Type, &param);
            VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_MaxFrameDelayCount, delayCount);
            CFRelease(delayCount);
        }
        #endif
        
        VTSessionSetProperty(m_CompressionSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
        
        VTCompressionSessionPrepareToEncodeFrames(m_CompressionSession);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoEncoder::unprepare(asio::yield_context &yctx)
    {
        VTCompressionSessionInvalidate(m_CompressionSession);
        CFRelease(m_CompressionSession);
        m_CompressionSession = 0;
        media::VideoEncoder::unprepare(yctx);
    }
    
    system::error_code H264VideoEncoder::resume(asio::yield_context &yctx)
    {
        if(system::error_code ee = VideoEncoder::resume(yctx))
            return ee;
        
        m_ForceVideoKeyFrame = true;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoEncoder::pause(asio::yield_context &yctx)
    {
        VideoEncoder::pause(yctx);
    }
    
    system::error_code H264VideoEncoder::run(asio::yield_context &yctx)
    {
        if(system::error_code ee = media::VideoEncoder::run(yctx))
            return ee;
        
        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
            return base::makeErrorCode(base::kEInvalidContent);

        intrusive_ptr<hal::av::CVImagePixelSampleBuffer> pixelSampleBuffer = dynamic_pointer_cast<hal::av::CVImagePixelSampleBuffer>(videoSample.buffer);

        if(!pixelSampleBuffer)
            return base::makeErrorCode(base::kEInvalidContent);

        m_VideoSampleTime = videoSample.time;
        
        
        CFMutableDictionaryRef frameProperties = CFDictionaryCreateMutable(NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        
        if(m_ForceVideoKeyFrame)
        {
            m_ForceVideoKeyFrame = false;
            CFDictionaryAddValue(frameProperties, kVTEncodeFrameOptionKey_ForceKeyFrame, kCFBooleanTrue);
        }
        
        CMTime pts = CMTimeMake(1000*m_VideoSampleTime, 1000);
        if(VTCompressionSessionEncodeFrame(
            m_CompressionSession,
            pixelSampleBuffer->imageBuffer(),
            pts,
            kCMTimeInvalid,
            frameProperties,
            pixelSampleBuffer->imageBuffer(),
            0
        ) != noErr)
        {
            CFRelease(frameProperties);
            return base::makeErrorCode(base::kEInvalidState);
        }
        
        VTCompressionSessionCompleteFrames(m_CompressionSession, kCMTimeInvalid);
        CFRelease(frameProperties);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoEncoder::rest(asio::yield_context &yctx)
    {
        m_pVideoOutputBuffer.reset();
        media::VideoEncoder::rest(yctx);
    }
    
    std::size_t H264VideoEncoder::numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<InputPort> H264VideoEncoder::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);
        
        return intrusive_ptr<InputPort>();
    }
    
    std::size_t H264VideoEncoder::numOutputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<OutputPort> H264VideoEncoder::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_VideoOutputPort);
        
        return intrusive_ptr<OutputPort>();
    }
    
    eCodecProfile H264VideoEncoder::getProfile() const
    {
        return m_Profile;
    }

    bool H264VideoEncoder::setProfile(eCodecProfile profile)
    {
        if(isRunning())
            return false;
        
        if(!(profile > 0 && profile < kCodecProfileMax))
            return false;
        
        m_Profile = profile;
        return true;
    }
    
    int H264VideoEncoder::getBitRate() const
    {
        return m_VideoBitRate;
    }
    
    bool H264VideoEncoder::setBitRate(int br)
    {
        if(isRunning())
            return false;
        
        m_VideoBitRate = br;
        return true;
    }
    
    
    eVideoCodecType H264VideoEncoder::getVideoCodecType() const
    {
        return kVideoCodecTypeH264;
    }
    
    SampleType H264VideoEncoder::getOutputPortSampleType() const
    {
        return SampleType(getVideoCodecType());
    }
    
    Sample H264VideoEncoder::getOutputPortSample() const
    {
        return Sample(m_pVideoOutputBuffer, m_VideoSampleTime);
    }
    
    float H264VideoEncoder::getOutputPortSampleRate() const
    {
        return m_VideoInputPort.getSampleRate();
    }
    
    Vec2i H264VideoEncoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoInputPort.getVideoSampleSize();
    }
    
    bool H264VideoEncoder::acceptsInputPortPixelFormat(base::ePixelFormat fmt) const
    {
        if(!fmt)
            return false;
        
        return true;
    }
    
    void H264VideoEncoder::vtCompressionOutputCallback(void *frame, OSStatus status, VTEncodeInfoFlags iflags, CMSampleBufferRef sb)
    {
        m_pVideoOutputBuffer.reset(new VideoOutputBuffer(sb) );
    }
    
    void H264VideoEncoder::_vtCompressionOutputCallback(void *_this, void *frame, OSStatus status, VTEncodeInfoFlags iflags, CMSampleBufferRef sb)
    {
        reinterpret_cast<H264VideoEncoder*>(_this)->vtCompressionOutputCallback(frame, status, iflags, sb);
    }
    
    
} //namespace vt
} //namespace media
} //namespace uquad