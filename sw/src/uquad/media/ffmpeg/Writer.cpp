#include "Writer.h"
#include "../Error.h"

namespace uquad
{
namespace media
{
namespace ffmpeg
{
	Writer::Registry::Registry(std::string const &n, ::AVOutputFormat *f)
		: media::Writer::Registry(n)
        , m_FFMpegOutputFormat(f)
        , m_ContainerFormat(FFMpeg::containerFormatFromFFMpegName(m_FFMpegOutputFormat->name))
	{

	}

	Writer::Registry::~Registry()
	{
	}
    
    eContainerFormat Writer::Registry::getContainerFormat() const
    {
        return m_ContainerFormat;
    }

	intrusive_ptr<media::Component> Writer::Registry::createComponent() const
	{
		return intrusive_ptr<media::Component>(new Writer(*this));
	}
    
    Writer::VideoInputPort::VideoInputPort(Writer &w)
        : media::VideoInputPort()
        , m_Writer(w)
    {
    }
    
    Writer::VideoInputPort::~VideoInputPort()
    {
    }
    
    intrusive_ptr<Sink> Writer::VideoInputPort::getSink() const
    {
        return intrusive_ptr<Sink>(&m_Writer);
    }
    
    bool Writer::VideoInputPort::accepts(intrusive_ptr<OutputPort> const &op) const
    {
        SampleType outSampleType = op->getSampleType();
        if(outSampleType.type() != kSampleTypeVideo)
            return false;
        
        return m_Writer.acceptsVideoSampleKind(outSampleType.videoKind());
        
    }
    

	Writer::Writer(Registry const &r)
		: media::Writer()
        , m_FFMpegOutputFormat(r.m_FFMpegOutputFormat)
        , m_ContainerFormat(r.m_ContainerFormat)
        , m_VideoInputPort(*this)
        , m_FFMpegFormatCtx(0)
        , m_FFMpegBuffer()
        , m_FFMpegIOContext(0)
        , m_FFMpegVideoStream(0)
        , m_TestFile()
    {
        intrusive_ptr_add_ref(&m_VideoInputPort);
	}

	Writer::~Writer()
	{
	}

	bool Writer::isValid() const
	{
		return m_FFMpegOutputFormat != 0;
	}
    
    bool Writer::prepareExecuting()
    {
        if(!media::Writer::prepareExecuting())
            return false;
        
        m_FFMpegBuffer.resize(8192);
        m_FFMpegIOContext = ::avio_alloc_context(
            m_FFMpegBuffer.data(), m_FFMpegBuffer.size(),
            1, this,
            &Writer::_ffmpegReadPacket,
            &Writer::_ffmpegWritePacket,
            &Writer::_ffmpegSeek
        );
        
        if(!m_FFMpegIOContext)
        {
            m_FFMpegBuffer.resize(0);
            return false;
        }
        
        if(::avformat_alloc_output_context2(&m_FFMpegFormatCtx, m_FFMpegOutputFormat, 0, 0) < 0)
        {
            ::av_freep(&m_FFMpegIOContext);
            m_FFMpegIOContext = 0;
            
            m_FFMpegBuffer.resize(0);
            return false;
        }
        
        
        if(m_ContainerFormat == kContainerFormatMPEG4)
        {
            //FFMpeg::av_opt_set_flag(m_FFMpegFormatCtx->priv_data, "frag_keyframe", "movflags");
        }
        
        
        m_FFMpegFormatCtx->pb = m_FFMpegIOContext;
        //m_FFMpegFormatCtx->flags |= AVFMT_FLAG_CUSTOM_IO;
        
        
        ::AVCodecID srcCodecId = FFMpeg::avCodecIdFromVideoSampleKind(m_VideoInputPort.getSampleType().videoKind());
        
        
        m_FFMpegVideoStream = ::avformat_new_stream(m_FFMpegFormatCtx, ::avcodec_find_encoder(srcCodecId));
        
        
        
        if(!m_FFMpegVideoStream)
        {
            ::avformat_free_context(m_FFMpegFormatCtx);
            m_FFMpegFormatCtx = 0;
            
            ::av_freep(&m_FFMpegIOContext);
            m_FFMpegIOContext = 0;
            
            m_FFMpegBuffer.resize(0);
            return false;
        }
        
        m_FFMpegVideoStream->id = m_FFMpegFormatCtx->nb_streams-1;
        
        ::AVCodecContext *codecCtx = m_FFMpegVideoStream->codec;
        //codecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
        
        Vec2i const &videoSize = m_VideoInputPort.getVideoSampleSize();
        float const videoSampleRate = m_VideoInputPort.getSampleRate();
        
        codecCtx->codec_id = srcCodecId;
        codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        codecCtx->width = videoSize(0);
        codecCtx->height = videoSize(1);
        codecCtx->time_base = (AVRational){1,static_cast<int>(videoSampleRate)};
        codecCtx->bit_rate = 400000;
        codecCtx->gop_size = 5;
        codecCtx->max_b_frames = 1;
        codecCtx->profile = FF_PROFILE_H264_HIGH;
        codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        //codecCtx->level = 41;
        codecCtx->sample_aspect_ratio = m_FFMpegVideoStream->sample_aspect_ratio;
        m_FFMpegVideoStream->time_base = codecCtx->time_base;
        m_FFMpegVideoStream->r_frame_rate = ::av_inv_q(codecCtx->time_base);
        m_FFMpegVideoStream->start_time = AV_NOPTS_VALUE;
        
        m_TestFile.open("test.mp4");
        ::avformat_write_header(m_FFMpegFormatCtx, 0);
        
        return true;
    }
    
    void Writer::doneExecuting()
    {
        media::Writer::doneExecuting();
        
        av_write_trailer(m_FFMpegFormatCtx);
        
        if(m_TestFile.is_open())
        {
            m_TestFile.flush();
            m_TestFile.close();
        }
        
        ::avcodec_close(m_FFMpegVideoStream->codec);
        m_FFMpegVideoStream = 0;
        
        ::avformat_free_context(m_FFMpegFormatCtx);
        m_FFMpegFormatCtx = 0;
        
        ::av_freep(&m_FFMpegIOContext);
        m_FFMpegIOContext = 0;
        
        m_FFMpegBuffer.resize(0);
    }
    
    bool Writer::execute()
    {
        intrusive_ptr<base::Buffer> videoInputBuffer = m_VideoInputPort.getSampleData();
        if(!videoInputBuffer || !videoInputBuffer->isValid())
            return true;
        
        m_VideoSampleTime = m_VideoInputPort.getSampleTime();
        
        #if 1
        ::AVPacket encodedPacket = { 0 };
        ::av_init_packet(&encodedPacket);
        encodedPacket.data = reinterpret_cast<uint8_t*>(videoInputBuffer->data());
        encodedPacket.size = videoInputBuffer->size();
        encodedPacket.stream_index = m_FFMpegVideoStream->index;
        encodedPacket.pts = static_cast<int64_t>(m_VideoSampleTime*m_FFMpegVideoStream->time_base.den);
        //encodedPacket.dts = encodedPacket.pts;
        return ::av_interleaved_write_frame(m_FFMpegFormatCtx, &encodedPacket) == 0;
        #else
        ::AVPacket *encodedPacket = reinterpret_cast<::AVPacket *>(videoInputBuffer->data());
        encodedPacket->stream_index = m_FFMpegVideoStream->index;
        return ::av_write_frame(m_FFMpegFormatCtx, encodedPacket) == 0;
        #endif
    }
    
    void Writer::postExecute()
    {
    }
        
    std::size_t Writer::numInputPorts() const
    {
        return 1;
    }
    
    intrusive_ptr<InputPort> Writer::getInputPort(std::size_t index) const
    {
        if(index == 0)
            return intrusive_ptr<InputPort>(&m_VideoInputPort);
        
        return intrusive_ptr<InputPort>();
    }
    
    eContainerFormat Writer::getContainerFormat() const
    {
        return m_ContainerFormat;
    }
    
    bool Writer::acceptsVideoSampleKind(eVideoSampleKind kind) const
    {
        if(!kind)
            return false;
        
        ::AVCodecID kindCodecId = FFMpeg::avCodecIdFromVideoSampleKind(kind);
        
        if(kindCodecId == AV_CODEC_ID_NONE)
            return false;
        
        if(m_FFMpegOutputFormat->query_codec)
        {
            return m_FFMpegOutputFormat->query_codec(kindCodecId, 1) != 0;
        }
        
        switch(m_ContainerFormat)
        {
        case  kContainerFormatMOV:
            break;
        case kContainerFormatMPEG4:
            {
                switch(kind)
                {
                case kVideoSampleKindH263:
                case kVideoSampleKindH264:
                case kVideoSampleKindMPEG4:
                    return true;
                    break;
                    
                default:
                    break;
                }
            }
            break;
        
        case kContainerFormatMPEGTS:
            break;
            
        default:
            break;
        }
        return m_FFMpegOutputFormat->video_codec == kindCodecId;
    }
    
    int Writer::_ffmpegReadPacket(void *opaque, uint8_t *buf, int buf_size)
    {
        Writer *pWriter = reinterpret_cast<Writer*>(opaque);
        if(pWriter && pWriter->m_TestFile.is_open())
        {
            pWriter->m_TestFile.read(reinterpret_cast<char*>(buf), buf_size);
            if(pWriter->m_TestFile)
                return buf_size;
            
            return pWriter->m_TestFile.gcount();
        }
        return 0;
    }
    
    int Writer::_ffmpegWritePacket(void *opaque, uint8_t *buf, int buf_size)
    {
        Writer *pWriter = reinterpret_cast<Writer*>(opaque);
        if(pWriter && pWriter->m_TestFile.is_open())
        {
            pWriter->m_TestFile.write(reinterpret_cast<const char*>(buf), buf_size);
        }
        return 0;
    }
    
    int64_t Writer::_ffmpegSeek(void *opaque, int64_t offset, int whence)
    {
        Writer *pWriter = reinterpret_cast<Writer*>(opaque);
        if(pWriter && pWriter->m_TestFile.is_open())
        {
            pWriter->m_TestFile.flush();
            
            std::ios_base::seekdir way;
            switch(whence)
            {
            case SEEK_SET: way = std::ios_base::beg; break;
            case SEEK_CUR: way = std::ios_base::cur; break;
            case SEEK_END: way = std::ios_base::end; break;
            default:
                return pWriter->m_TestFile.tellp();
                break;
            }
            
            pWriter->m_TestFile.seekp(offset, way);
            return pWriter->m_TestFile.tellp();
        }
        return 0;
    }
    

} //namespace omx
} //namespace media
} //namespace uquad
