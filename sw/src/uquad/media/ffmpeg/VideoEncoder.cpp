#include "VideoEncoder.h"
#include "../../base/Error.h"
#include "FFMpeg.h"
#include "../../utility/Constant.h"
#include "../../base/PixelSampleBuffer.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	VideoEncoder::Registry::Registry(std::string const &n, ::AVCodec *c)
		: media::VideoEncoder::Registry(n)
        , m_FFMpegCodec(c)
	{

	}

	VideoEncoder::Registry::~Registry()
	{
	}

    eVideoCodecType VideoEncoder::Registry::getVideoCodecType() const
    {
        return FFMpeg::videoCodecTypeFromAVCodecId(m_FFMpegCodec->id);
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
    
    VideoEncoder::VideoEncoder(Registry const &r)
		: media::VideoEncoder()
        , m_FFMpegCodec(r.m_FFMpegCodec)
        , m_FFMpegCodecCtx(0)
        , m_VideoInputPort(*this)
        , m_VideoOutputPort(*this)
        , m_VideoOutputBuffer()
        , m_pVideoOutputBuffer(&m_VideoOutputBuffer)
        , m_VideoFrameConverter(0)
        , m_VideoFrameTransform()
        , m_VideoSampleTime(0.0f)
        , m_Profile(kCodecProfileLowLatency)
        , m_VideoBitRate(400000)
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
        intrusive_ptr_add_ref(&m_VideoOutputPort);
        intrusive_ptr_add_ref(&m_VideoOutputBuffer);
	}

	VideoEncoder::~VideoEncoder()
	{
	}

	bool VideoEncoder::isValid() const
	{
		return media::VideoEncoder::isValid() && (m_FFMpegCodec != 0);
	}
    
    system::error_code VideoEncoder::prepare(asio::yield_context &yctx)
    {
        if(system::error_code e = media::VideoEncoder::prepare(yctx))
            return e;
        
        m_FFMpegCodecCtx = ::avcodec_alloc_context3(m_FFMpegCodec);
        if(!m_FFMpegCodecCtx)
            return base::makeErrorCode(base::kEInvalidContent);
        
        
        ::avcodec_get_context_defaults3(m_FFMpegCodecCtx, m_FFMpegCodec);
        
        Vec2i videoSize = m_VideoInputPort.getVideoSampleSize();
        SampleType videoSampleType = m_VideoInputPort.getSampleType();
        ::AVPixelFormat srcFormat = FFMpeg::avPixelFormatFromPixelFormat(videoSampleType.kind.videoRaw);
        ::AVPixelFormat dstFormat = m_FFMpegCodec->pix_fmts[0];
        m_VideoFrameConverter = ::sws_getContext(
            videoSize(0), videoSize(1), srcFormat,
            videoSize(0), videoSize(1), dstFormat,
            SWS_POINT, NULL, NULL, NULL
        );
        
        if(!m_VideoFrameConverter)
        {
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        ::avcodec_get_frame_defaults(&m_VideoFrameTransform);
        
        m_VideoFrameTransform.format = dstFormat;
        m_VideoFrameTransform.width = videoSize(0);
        m_VideoFrameTransform.height = videoSize(1);
        
        if(::av_image_alloc(
            m_VideoFrameTransform.data,
            m_VideoFrameTransform.linesize,
            m_VideoFrameTransform.width,
            m_VideoFrameTransform.height,
            dstFormat,
            32) < 0)
        {
            ::sws_freeContext(m_VideoFrameConverter);
            m_VideoFrameConverter = 0;
            
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        
        float videoSampleRate = m_VideoInputPort.getSampleRate();
        
        m_FFMpegCodecCtx->width = videoSize(0);
        m_FFMpegCodecCtx->height = videoSize(1);
        m_FFMpegCodecCtx->time_base = (AVRational){1,static_cast<int>(videoSampleRate)};
        m_FFMpegCodecCtx->bit_rate = m_VideoBitRate;
        //m_FFMpegCodecCtx->gop_size = 0;
        //m_FFMpegCodecCtx->max_b_frames = 0;
        m_FFMpegCodecCtx->pix_fmt = dstFormat;
        
        //m_FFMpegCodecCtx->profile = FF_PROFILE_H264_BASELINE;
        //m_FFMpegCodecCtx->level = 41;
        
        if(m_FFMpegCodecCtx->codec_id == ::AV_CODEC_ID_H264)
        {
            ::av_opt_set(m_FFMpegCodecCtx->priv_data, "preset", "ultrafast", 0);
            ::av_opt_set(m_FFMpegCodecCtx->priv_data, "tune", "zerolatency", 0);
            //::av_opt_set(m_FFMpegCodecCtx->priv_data, "g", "30", 0);
        }
        
        if(::avcodec_open2(m_FFMpegCodecCtx, m_FFMpegCodec, NULL) < 0)
        {
            ::sws_freeContext(m_VideoFrameConverter);
            m_VideoFrameConverter = 0;
            
            av_freep(&m_VideoFrameTransform.data[0]);
            
            ::avcodec_close(m_FFMpegCodecCtx);
            m_FFMpegCodecCtx = 0;
            return base::makeErrorCode(base::kEInvalidContent);
        }
        m_VideoSampleTime = 0.0f;
        return base::makeErrorCode(base::kENoError);
    }
    
    void VideoEncoder::unprepare()
    {
        av_freep(&m_VideoFrameTransform.data[0]);
        sws_freeContext(m_VideoFrameConverter);
        m_VideoFrameConverter = 0;
        
        ::avcodec_close(m_FFMpegCodecCtx);
        m_FFMpegCodecCtx = 0;
        media::VideoEncoder::unprepare();
    }
    
    system::error_code VideoEncoder::run(asio::yield_context &yctx)
    {
        if(system::error_code ee = media::VideoEncoder::run(yctx))
            return ee;

        Sample videoSample = m_VideoInputPort.getSample();
        if(!videoSample)
            return base::makeErrorCode(base::kEInvalidContent);
        
        
        intrusive_ptr<base::PixelSampleBuffer> pixelSampleBuffer = dynamic_pointer_cast<base::PixelSampleBuffer>(videoSample.buffer);
        
        if(!pixelSampleBuffer || pixelSampleBuffer->lock())
            return base::makeErrorCode(base::kEInvalidContent);
        
        //Vec2i pixelSampleBufferSize = pixelSampleBuffer->planeSize();
        
        
        ::AVPixelFormat srcFormat = FFMpeg::avPixelFormatFromPixelFormat(pixelSampleBuffer->format());
        ::AVFrame frame;
        avcodec_get_frame_defaults(&frame);
        
        frame.format = srcFormat;
        frame.width = m_FFMpegCodecCtx->width;
        frame.height = m_FFMpegCodecCtx->height;
        
        std::size_t pixelSampleBufferPlaneCount = pixelSampleBuffer->planeCount();
        for(std::size_t ip = 0; ip < pixelSampleBufferPlaneCount; ++ip)
        {
            frame.data[ip] = reinterpret_cast<uint8_t*>(pixelSampleBuffer->planeData(ip));
            frame.linesize[ip] = pixelSampleBuffer->planeRowBytes(ip);
        }
        for(std::size_t ip = pixelSampleBufferPlaneCount; ip < 4; ++ip)
        {
            frame.data[ip] = 0;
            frame.linesize[ip] = 0;
        }
        
        if(::sws_scale(m_VideoFrameConverter,
            frame.data, frame.linesize, 0, frame.height,
            m_VideoFrameTransform.data, m_VideoFrameTransform.linesize) < 0)
        {
            pixelSampleBuffer->unlock();
            return base::makeErrorCode(base::kEInvalidContent);
        }
        pixelSampleBuffer->unlock();
        
        m_VideoFrameTransform.pts = static_cast<int64_t>(videoSample.time*m_FFMpegCodecCtx->time_base.den);
        
        m_VideoOutputBuffer.init();
        
        m_VideoOutputBuffer.m_AVPacket.data = 0;
        m_VideoOutputBuffer.m_AVPacket.size = 0;
        
        int frameEncoded = 0;
        
        if(::avcodec_encode_video2(m_FFMpegCodecCtx, &m_VideoOutputBuffer.m_AVPacket, &m_VideoFrameTransform, &frameEncoded) < 0)
            return base::makeErrorCode(base::kEInvalidContent);
        
        if(frameEncoded)
        {
            m_VideoSampleTime = videoSample.time;
            m_pVideoOutputBuffer.reset(&m_VideoOutputBuffer);
            //std::cout << "frame: " << m_VideoOutputBuffer.m_AVPacket.size << std::endl;
        }
        else
        {
            m_pVideoOutputBuffer.reset();
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    void VideoEncoder::rest()
    {
        m_VideoOutputBuffer.release();
        m_pVideoOutputBuffer.reset();
        media::VideoEncoder::rest();
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
        return FFMpeg::videoCodecTypeFromAVCodecId(m_FFMpegCodec->id);
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
        return m_VideoInputPort.getSampleRate();
    }
    
    Vec2i VideoEncoder::getOutputPortVideoSampleSize() const
    {
        return m_VideoInputPort.getVideoSampleSize();
    }
    
    bool VideoEncoder::acceptsInputPortPixelFormat(base::ePixelFormat fmt) const
    {
        if(!fmt)
            return false;
        
        return true;
    }

} //namespace omx
} //namespace media
} //namespace uquad
