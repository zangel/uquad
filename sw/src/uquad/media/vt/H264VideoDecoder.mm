#include "H264VideoDecoder.h"
#include "../../hal/av/CVImagePixelSampleBuffer.h"
#include "../../base/Image.h"
#include "../../base/Error.h"

#include "../../base/Application.h"


namespace uquad
{
namespace media
{
namespace vt
{
    namespace
    {
        struct NALU
        {
            typedef enum
            {
                kUnspecified = 0,
                kNonIDRSlice = 1,
                kSliceDataA = 2,
                kSliceDataB = 3,
                kSliceDataC = 4,
                kIDRSlice = 5,
                kSEIMessage = 6,
                kSPS = 7,
                kPPS = 8,
                kAUD = 9,
                kEOSeq = 10,
                kEOStream = 11,
                kFiller = 12,
                kSPSExt = 13,
                kReserved14 = 14,
                kReserved15 = 15,
                kReserved16 = 16,
                kReserved17 = 17,
                kReserved18 = 18,
                kCodedSliceAux = 19,
                kCodedSliceExtension = 20,
                kCodecSliceExtensionForDVC = 21,
                kReserved22 = 22,
                kReserved23 = 23,
                kStapA = 24,
                kStapB = 25,
                kMTAP16 = 26,
                kMTAP24 = 27,
                kFU_A = 28,
                kFU_B = 29,
                kUnspecified30 = 30,
                kUnspecified31 = 31
            } eType;
            
            NALU() : data(0), size(0), type(kUnspecified), ref_idc(0) {}
            
            inline operator bool() const { return data != 0 && type != kUnspecified; }
            inline bool operator !() const { return data == 0 || type == kUnspecified; }
  
            uint8_t const *data;
            std::size_t size;
            
            eType type;
            int ref_idc;
        };
        
        class NALUParser
        {
        public:
            typedef enum
            {
                kOk = 0,
                kInvalidStream,
                kUnsupportedStream,
                kEOStream
            } eResult;
  
            NALUParser(uint8_t const *data, std::size_t size)
                : m_Stream(data)
                , m_BytesLeft(size)
            {
            }
            
            eResult advanceToNextNALU(NALU &nalu)
            {
                std::size_t startCodeSize;
                std::size_t naluSizeWithStartCode;
                
                if(!locateNALU(naluSizeWithStartCode, startCodeSize))
                {
                    return kEOStream;
                }
                
                nalu.data = m_Stream + startCodeSize;
                nalu.size = naluSizeWithStartCode - startCodeSize;
                
                m_Stream += naluSizeWithStartCode;
                m_BytesLeft -= naluSizeWithStartCode;
                
                if(!nalu.size)
                    return kEOStream;
                
                if(nalu.data[0] & 0x80)
                    return kInvalidStream;
                
                nalu.ref_idc = (nalu.data[0] & 0x60) >> 5;
                nalu.type = NALU::eType(nalu.data[0] & 0x1F);
                
                return kOk;
            }
            
            
        private:
            bool locateNALU(std::size_t &nsize, std::size_t &scsize)
            {
                std::size_t naluStartOffset = 0;
                std::size_t startCodeSize = 0;
                
                if(!findStartCode(m_Stream, m_BytesLeft, naluStartOffset, startCodeSize))
                    return false;
                
                m_Stream += naluStartOffset;
                m_BytesLeft -= naluStartOffset;
                
                uint8_t const *naluData = m_Stream + startCodeSize;
                std::ptrdiff_t naluMaxDataSize = m_BytesLeft - startCodeSize;
                
                if(naluMaxDataSize <= 0)
                {
                    return false;
                }

                std::size_t nextStartCodeSize = 0;
                std::size_t naluSizeWithoutStartCode = 0;
                if(!findStartCode(naluData, naluMaxDataSize, naluSizeWithoutStartCode, nextStartCodeSize))
                    naluSizeWithoutStartCode = naluMaxDataSize;
                
                nsize = naluSizeWithoutStartCode + startCodeSize;
                scsize = startCodeSize;
                return true;
            }
            
            static inline bool isStartCode(uint8_t const *data)
            {
                return data[0] == 0x00 && data[1] == 0x00 && data[2] == 0x01;
            }
            
            static inline bool findStartCode(uint8_t const *data, std::size_t size, std::size_t &offset, std::size_t &scsize)
            {
                std::size_t bytesLeft = size;
                while(bytesLeft >= 3)
                {
                    if(isStartCode(data))
                    {
                        offset = size - bytesLeft;
                        scsize = 3;
                        if(offset > 0 && *(data-1) == 0x00)
                        {
                            --offset;
                            ++scsize;
                        }
                        return true;
                    }
                    ++data;
                    --bytesLeft;
                }
                offset = size - bytesLeft;
                scsize = 0;
                return false;
            }
            
            
            
        private:
            uint8_t const *m_Stream;
            std::size_t m_BytesLeft;
        };
        
        
    }
    
    H264VideoDecoder::Registry::Registry(std::string const &n)
		: media::VideoDecoder::Registry(n)
	{

	}

	H264VideoDecoder::Registry::~Registry()
	{
	}

    eVideoCodecType H264VideoDecoder::Registry::getVideoCodecType() const
    {
        return kVideoCodecTypeH264;
    }
    
	intrusive_ptr<media::Component> H264VideoDecoder::Registry::createComponent() const
	{
		return intrusive_ptr<media::Component>(new H264VideoDecoder(*this));
	}
    
    H264VideoDecoder::VideoInputPort::VideoInputPort(H264VideoDecoder &vd)
        : media::VideoInputPort()
        , m_VideoDecoder(vd)
    {
    }
    
    H264VideoDecoder::VideoInputPort::~VideoInputPort()
    {
    }
    
    intrusive_ptr<Sink> H264VideoDecoder::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_VideoDecoder);
    }
    
    bool H264VideoDecoder::VideoInputPort::acceptsPort(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type != kSampleTypeVideoEncoded)
            return false;
        
        return m_VideoDecoder.acceptsInputPortVideoCodecType(outSampleType.kind.videoEncoded);
    }
    
    H264VideoDecoder::VideoOutputPort::VideoOutputPort(H264VideoDecoder &vd)
        : media::VideoOutputPort()
        , m_VideoDecoder(vd)
    {
    }
    
    H264VideoDecoder::VideoOutputPort::~VideoOutputPort()
    {
    }
    
    intrusive_ptr<Source> H264VideoDecoder::VideoOutputPort::getSource() const
    {
        return intrusive_ptr<Source>(&m_VideoDecoder);
    }
    
    
    SampleType H264VideoDecoder::VideoOutputPort::getSampleType() const
    {
        return m_VideoDecoder.getOutputPortSampleType();
    }
    
    Sample H264VideoDecoder::VideoOutputPort::getSample() const
    {
        return m_VideoDecoder.getOutputPortSample();
    }
    
    float H264VideoDecoder::VideoOutputPort::getSampleRate() const
    {
        return m_VideoDecoder.getOutputPortSampleRate();
    }
    
    Vec2i H264VideoDecoder::VideoOutputPort::getVideoSampleSize() const
    {
        return m_VideoDecoder.getOutputPortVideoSampleSize();
    }
    
    H264VideoDecoder::H264VideoDecoder(Registry const &r)
		: media::VideoDecoder()
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_VideoOutputFormat(base::kPixelFormatRGB8)
        , m_VTDSession(0)
        , m_CMFormatDesc(0)
        , m_pVideoOutputBuffer()
        , m_VideoBitRate(400000)
        , m_VideoSampleSize(0,0)
        , m_VideoSampleRate(0.0f)
        , m_VideoSampleTime(0.0f)
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);
	}

	H264VideoDecoder::~H264VideoDecoder()
	{
	}

	bool H264VideoDecoder::isValid() const
	{
		return media::VideoDecoder::isValid();
	}
    
    system::error_code H264VideoDecoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code e = media::VideoDecoder::prepare(yctx))
            return e;
        
        m_VideoSampleSize = m_VideoInputPort.getVideoSampleSize();
        m_VideoSampleRate = m_VideoInputPort.getSampleRate();
        m_VideoSampleTime = 0.0f;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoDecoder::unprepare(asio::yield_context &yctx)
    {
        m_pVideoOutputBuffer.reset();
        
        if(m_VTDSession)
        {
            VTDecompressionSessionInvalidate(m_VTDSession);
            CFRelease(m_VTDSession);
            m_VTDSession = 0;
        }
        
        if(m_CMFormatDesc)
        {
            CFRelease(m_CMFormatDesc);
            m_CMFormatDesc = 0;
        }
        media::VideoDecoder::unprepare(yctx);
    }
    
    system::error_code H264VideoDecoder::resume(asio::yield_context &yctx)
    {
        if(system::error_code de = VideoDecoder::resume(yctx))
            return de;
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoDecoder::pause(asio::yield_context &yctx)
    {
        VideoDecoder::pause(yctx);
    }
    
    
    system::error_code H264VideoDecoder::run(asio::yield_context &yctx)
    {
        if(system::error_code de = media::VideoDecoder::run(yctx))
            return de;

        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
            return base::makeErrorCode(base::kEInvalidContent);
        
        
        NALUParser naluParser(
            reinterpret_cast<uint8_t const *>(videoSample.buffer->data()),
            videoSample.buffer->size()
        );
        
        NALU sps, pps;
        std::vector<NALU> nalus;
        std::size_t nalusDataSize = 0;
        
        
        NALUParser::eResult result;
        
        while(true)
        {
            NALU nalu;
            result = naluParser.advanceToNextNALU(nalu);
            
            if(result == NALUParser::kEOStream || result != NALUParser::kOk)
                break;
            
            switch(nalu.type)
            {
            case NALU::kSPS:
                sps = nalu;
                break;
                
            case NALU::kPPS:
                pps = nalu;
                break;
            
            default:
                nalus.push_back(nalu);
                nalusDataSize += sizeof(uint32_t) + nalu.size;
            }
        }
        
        if(result == NALUParser::kInvalidStream)
            return base::makeErrorCode(base::kEInvalidContent);
        
        if(!sps != !pps)
            return base::makeErrorCode(base::kEInvalidContent);
        
        if(sps && pps)
        {
            if(m_CMFormatDesc)
            {
                CFRelease(m_CMFormatDesc);
                m_CMFormatDesc = 0;
            }
            
            uint8_t const*  psetPtrs[2] = { sps.data, pps.data };
            std::size_t psetSizes[2] = { sps.size, pps.size };
            
            if(CMVideoFormatDescriptionCreateFromH264ParameterSets(
                kCFAllocatorDefault,
                2,
                psetPtrs,
                psetSizes,
                4,
                &m_CMFormatDesc
            ) != noErr)
            {
                return base::makeErrorCode(base::kEInvalidContent);
            }
            
            if(m_VTDSession)
            {
                if(!VTDecompressionSessionCanAcceptFormatDescription(m_VTDSession, m_CMFormatDesc))
                {
                    VTDecompressionSessionInvalidate(m_VTDSession);
                    CFRelease(m_VTDSession);
                    m_VTDSession = 0;
                }
            }
        }
        
        if(!m_VTDSession)
        {
            VTDecompressionOutputCallbackRecord callback;
            callback.decompressionOutputCallback = &H264VideoDecoder::_vtDecodeCallback;
            callback.decompressionOutputRefCon = this;
            
            #if defined(UQUAD_PLATFORM_Mac)
            
            CFMutableDictionaryRef imgCfg = CFDictionaryCreateMutable(kCFAllocatorDefault, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
            
            #if 1
            int32_t pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;//kCVPixelFormatType_422YpCbCr8;
            #else
            int32_t pixelFormat = kCVPixelFormatType_32BGRA;
            #endif
            CFNumberRef imgCfgPixelFormat =  CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixelFormat);
            CFDictionarySetValue(imgCfg, kCVPixelBufferPixelFormatTypeKey, imgCfgPixelFormat);
            CFRelease(imgCfgPixelFormat);
            
            CMVideoDimensions codedImageSize = CMVideoFormatDescriptionGetDimensions(m_CMFormatDesc);
            CFNumberRef imgCfgWidth =  CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &codedImageSize.width);
            CFDictionarySetValue(imgCfg, kCVPixelBufferWidthKey, imgCfgWidth);
            CFRelease(imgCfgWidth);
            CFNumberRef imgCfgHeight =  CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &codedImageSize.height);
            CFDictionarySetValue(imgCfg, kCVPixelBufferHeightKey, imgCfgHeight);
            CFRelease(imgCfgHeight);
            
            CFDictionarySetValue(imgCfg, kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);
            #else
            CFMutableDictionaryRef imgCfg = 0;
            #endif
            
            
            if(VTDecompressionSessionCreate(
                0,
                m_CMFormatDesc,
                0,
                imgCfg,
                &callback,
                &m_VTDSession) != noErr)
            {
                if(imgCfg) CFRelease(imgCfg);
                return base::makeErrorCode(base::kEInvalidContent);
            }
            if(imgCfg) CFRelease(imgCfg);
        }
        
        if(!nalusDataSize)
        {
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        CMBlockBufferRef blockBuffer = 0;
        if(CMBlockBufferCreateWithMemoryBlock(
            kCFAllocatorDefault,
            0,
            nalusDataSize,
            kCFAllocatorDefault,
            0,
            0,
            nalusDataSize,
            0,
            &blockBuffer) != noErr)
        {
            return base::makeErrorCode(base::kEInvalidContent);
        }
        CMBlockBufferAssureBlockMemory(blockBuffer);
        
        std::size_t offset = 0;
        
        for(std::size_t i = 0; i < nalus.size(); i++)
        {
            NALU &nalu = nalus[i];
            
            uint32_t header = htonl(nalu.size);
            CMBlockBufferReplaceDataBytes(
                &header,
                blockBuffer,
                offset,
                sizeof(header)
            );
            offset += sizeof(header);
            CMBlockBufferReplaceDataBytes(
                nalu.data,
                blockBuffer,
                offset,
                nalu.size
            );
            offset += nalu.size;
        }
        
        CMSampleBufferRef sampleBuffer = 0;
        if(CMSampleBufferCreate(
            kCFAllocatorDefault,
            blockBuffer,
            true,
            0,
            0,
            m_CMFormatDesc,
            1,
            0,
            0,
            0,
            0,
            &sampleBuffer) != noErr)
        {
            CFRelease(blockBuffer);
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(VTDecompressionSessionDecodeFrame(
            m_VTDSession,
            sampleBuffer,
            0,
            0,
            0) != noErr)
        {
            CFRelease(blockBuffer);
            CFRelease(sampleBuffer);
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        CFRelease(blockBuffer);
        CFRelease(sampleBuffer);
        
        return base::makeErrorCode(base::kENoError);
    }
    
    void H264VideoDecoder::rest(asio::yield_context &yctx)
    {
        m_pVideoOutputBuffer.reset();
        media::VideoDecoder::rest(yctx);
    }
    
    std::size_t H264VideoDecoder::numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<InputPort> H264VideoDecoder::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);
        
        return intrusive_ptr<InputPort>();
    }
    
    std::size_t H264VideoDecoder::numOutputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<OutputPort> H264VideoDecoder::getOutputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<OutputPort>(&m_VideoOutputPort);
        
        return intrusive_ptr<OutputPort>();
    }
    
    eCodecProfile H264VideoDecoder::getProfile() const
    {
        return m_Profile;
    }

    bool H264VideoDecoder::setProfile(eCodecProfile profile)
    {
        if(isRunning())
            return false;
        
        if(!(profile > 0 && profile < kCodecProfileMax))
            return false;
            
        m_Profile = profile;
        return true;
    }
        
    int H264VideoDecoder::getBitRate() const
    {
        return m_VideoBitRate;
    }
    
    bool H264VideoDecoder::setBitRate(int br)
    {
        if(isRunning())
            return false;
        
        m_VideoBitRate = br;
        return true;
    }
    
    
    eVideoCodecType H264VideoDecoder::getVideoCodecType() const
    {
        return kVideoCodecTypeH264;
    }
    
    bool H264VideoDecoder::acceptsInputPortVideoCodecType(eVideoCodecType vct) const
    {
        if(!vct)
            return false;
        
        return vct == getVideoCodecType();
    }
    
    SampleType H264VideoDecoder::getOutputPortSampleType() const
    {
        return SampleType(m_VideoOutputFormat);
    }
    
    Sample H264VideoDecoder::getOutputPortSample() const
    {
        return Sample(m_pVideoOutputBuffer, m_VideoSampleTime);
    }
    
    float H264VideoDecoder::getOutputPortSampleRate() const
    {
        return m_VideoSampleRate;
    }
    
    Vec2i H264VideoDecoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoSampleSize;
    }
    
    void H264VideoDecoder::vtDecodeCallback(void *frame, OSStatus status, VTDecodeInfoFlags iflags, CVImageBufferRef ib, CMTime pts, CMTime pd)
    {
        if(ib)
        {
            hal::av::CVImagePixelSampleBuffer *pCVImgBuffer = new hal::av::CVImagePixelSampleBuffer(ib);
            m_pVideoOutputBuffer.reset(pCVImgBuffer);
            
            #if 0
            if(!pCVImgBuffer->lock())
            {
                base::Image img((intrusive_ptr<base::PixelSampleBuffer>(pCVImgBuffer)));
                static int frame = 0;
                img.write((base::Application::dataDirectory()/(std::string("frame_") + lexical_cast<std::string>(frame++) + ".jpg")).string(), base::kImageFormatJPEG);
                
                pCVImgBuffer->unlock();
            }
            #endif
        }
    }
    
    void H264VideoDecoder::_vtDecodeCallback(void *_this, void *frame, OSStatus status, VTDecodeInfoFlags iflags, CVImageBufferRef ib, CMTime pts, CMTime pd)
    {
        reinterpret_cast<H264VideoDecoder*>(_this)->vtDecodeCallback(frame, status, iflags, ib, pts, pd);
    }
    
    
} //namespace ffmpeg
} //namespace media
} //namespace uquad