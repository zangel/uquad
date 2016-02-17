#include "VideoDecoder.h"
#include "../../base/Error.h"
#include "FFMpeg.h"
#include "../../utility/Constant.h"
#include "../../base/Image.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	VideoDecoder::Registry::Registry(std::string const &n, ::AVCodec *c)
		: media::VideoDecoder::Registry(n)
        , m_FFMpegCodec(c)
	{

	}

	VideoDecoder::Registry::~Registry()
	{
	}

    eVideoCodecType VideoDecoder::Registry::getVideoCodecType() const
    {
        return FFMpeg::videoCodecTypeFromAVCodecId(m_FFMpegCodec->id);
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
        , m_FFMpegCodec(r.m_FFMpegCodec)
        , m_FFMpegCodecCtx(0)
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_VideoOutputFormat(base::kPixelFormatRGB8)
        , m_VideoOutputBuffer()
        , m_pVideoOutputBuffer()
        , m_VideoFrameConverterSrcFormat(::AV_PIX_FMT_NONE)
        , m_VideoFrameConverter(0)
        , m_VideoSampleTime(0.0f)
        , m_Profile(kCodecProfileLowLatency)
        , m_VideoBitRate(400000)
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);
        intrusive_ptr_add_ref(&m_VideoOutputBuffer);
	}

	VideoDecoder::~VideoDecoder()
	{
	}

	bool VideoDecoder::isValid() const
	{
		return media::VideoDecoder::isValid() && (m_FFMpegCodec != 0);
	}
    
    system::error_code VideoDecoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code e = media::VideoDecoder::prepare(yctx))
            return e;
        
        m_FFMpegCodecCtx = ::avcodec_alloc_context3(m_FFMpegCodec);
        if(!m_FFMpegCodecCtx)
            return base::makeErrorCode(base::kEInvalidContent);
        
        ::avcodec_get_context_defaults3(m_FFMpegCodecCtx, m_FFMpegCodec);
        
        m_VideoFrameConverterSrcFormat = ::AV_PIX_FMT_YUV420P;
        Vec2i const &videoSize = m_VideoInputPort.getVideoSampleSize();

        ::AVPixelFormat dstFormat = FFMpeg::avPixelFormatFromPixelFormat(m_VideoOutputFormat);
        m_VideoFrameConverter = ::sws_getContext(
            videoSize(0), videoSize(1), m_VideoFrameConverterSrcFormat,
            videoSize(0), videoSize(1), dstFormat,
            SWS_POINT, NULL, NULL, NULL
        );
        
        if(!m_VideoFrameConverter)
        {
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(!m_VideoOutputBuffer.create(m_VideoOutputFormat, videoSize))
        {
            ::sws_freeContext(m_VideoFrameConverter);
            m_VideoFrameConverter = 0;
            
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        if(::avcodec_open2(m_FFMpegCodecCtx, m_FFMpegCodec, NULL) < 0)
        {
            ::sws_freeContext(m_VideoFrameConverter);
            m_VideoFrameConverter = 0;
            
            
            m_VideoOutputBuffer.release();
            
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        m_VideoSampleTime = 0.0f;
        return base::makeErrorCode(base::kENoError);
    }
    
    void VideoDecoder::unprepare()
    {
        m_VideoOutputBuffer.release();
        
        if(m_VideoFrameConverter)
        {
            ::sws_freeContext(m_VideoFrameConverter);
            m_VideoFrameConverter = 0;
        }
        
        ::avcodec_close(m_FFMpegCodecCtx);
        m_FFMpegCodecCtx = 0;
        media::VideoDecoder::unprepare();
    }
    
    system::error_code VideoDecoder::run(asio::yield_context &yctx)
    {
        if(system::error_code de = media::VideoDecoder::run(yctx))
            return de;
        
        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample || videoSample.buffer->lock())
            return base::makeErrorCode(base::kEInvalidContent);
        
        Vec2i videoSize = m_VideoInputPort.getVideoSampleSize();
        
        ::AVPacket packet;
        ::av_init_packet(&packet);
        packet.data = reinterpret_cast<uint8_t*>(videoSample.buffer->data());
        packet.size = videoSample.buffer->size();
        
        
        int frameDecoded = 0;
        ::AVFrame frame;
        ::avcodec_get_frame_defaults(&frame);
        
        int nDecoded = ::avcodec_decode_video2(m_FFMpegCodecCtx, &frame, &frameDecoded, &packet);
        if(nDecoded < 0)
        {
            videoSample.buffer->unlock();
            return base::makeErrorCode(base::kEInvalidContent);
        }
        videoSample.buffer->unlock();
        
        if(frameDecoded)
        {
            /*
            if(::av_image_fill_linesizes(frame.linesize, frame.format, frame.width) < 0)
                return false;
            
            if(::av_image_fill_pointers(frame.data, srcFormat, frame.height, reinterpret_cast<uint8_t*>(videoSample.buffer->data()), frame.linesize) < 0)
                return false;
            
            */
            
            if(m_VideoFrameConverterSrcFormat != frame.format)
            {
                ::sws_freeContext(m_VideoFrameConverter);
                ::AVPixelFormat dstFormat = FFMpeg::avPixelFormatFromPixelFormat(m_VideoOutputFormat);
                m_VideoFrameConverter = ::sws_getContext(
                    videoSize(0), videoSize(1), (::AVPixelFormat)frame.format,
                    videoSize(0), videoSize(1), dstFormat,
                    SWS_POINT, NULL, NULL, NULL
                );
                
                if(!m_VideoFrameConverter)
                {
                    ::avcodec_close(m_FFMpegCodecCtx);
                    m_FFMpegCodecCtx = 0;
                    return base::makeErrorCode(base::kEInvalidContent);
                }
                m_VideoFrameConverterSrcFormat = (::AVPixelFormat)frame.format;
            }
            
            if(::sws_scale(m_VideoFrameConverter,
                frame.data, frame.linesize, 0, frame.height,
                m_VideoOutputBuffer.m_AVFrame.data, m_VideoOutputBuffer.m_AVFrame.linesize) < 0)
            {
                return base::makeErrorCode(base::kEInvalidContent);
            }
        
            m_VideoSampleTime = videoSample.time;
            //m_VideoOutputBuffer.m_AVFrame.pts = static_cast<int64_t>(videoSample.time*m_FFMpegCodecCtx->time_base.den);
            m_pVideoOutputBuffer.reset(&m_VideoOutputBuffer);
            //std::cout << "frame: " << m_VideoOutputBuffer.m_AVPacket.size << std::endl;
            
            
            
            base::Image img((intrusive_ptr<base::PixelSampleBuffer>(&m_VideoOutputBuffer)));
            static int frame = 0;
            img.write(std::string("frame_") + lexical_cast<std::string>(frame++) + ".jpg", base::kImageFormatJPEG);
            
            
            
        }
        else
        {
            m_pVideoOutputBuffer.reset();
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    void VideoDecoder::rest()
    {
        m_pVideoOutputBuffer.reset();
        media::VideoDecoder::rest();
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
        return FFMpeg::videoCodecTypeFromAVCodecId(m_FFMpegCodec->id);
    }
    
    bool VideoDecoder::acceptsInputPortVideoCodecType(eVideoCodecType vct) const
    {
        if(!vct)
            return false;
        
        return vct == getVideoCodecType();
    }
    
    SampleType VideoDecoder::getOutputPortSampleType() const
    {
        return SampleType(m_VideoOutputFormat);
    }
    
    Sample VideoDecoder::getOutputPortSample() const
    {
        return Sample(m_pVideoOutputBuffer, m_VideoSampleTime);
    }
    
    float VideoDecoder::getOutputPortSampleRate() const
    {
        return m_VideoInputPort.getSampleRate();
    }
    
    Vec2i VideoDecoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoInputPort.getVideoSampleSize();
    }
    

} //namespace omx
} //namespace media
} //namespace uquad
