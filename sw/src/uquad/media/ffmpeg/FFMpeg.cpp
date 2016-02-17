#include "FFMpeg.h"
#include "../../base/Error.h"
#include "VideoEncoder.h"
#include "VideoDecoder.h"


namespace uquad
{
namespace media
{
namespace ffmpeg
{
	FFMpeg::FFMpeg()
	{
        ::av_log_set_level(AV_LOG_ERROR);
        //::av_log_set_level(AV_LOG_VERBOSE);
        ::avcodec_register_all();
        ::av_register_all();
        m_bInitialized = true;
	}

	FFMpeg::~FFMpeg()
	{
		m_bInitialized = false;
	}
    
    eVideoCodecType FFMpeg::videoCodecTypeFromAVCodecId(::AVCodecID cid)
    {
        switch(cid)
        {
        case ::AV_CODEC_ID_H263:  return kVideoCodecTypeH263; break;
        case ::AV_CODEC_ID_H264:  return kVideoCodecTypeH264; break;
        case ::AV_CODEC_ID_MPEG4: return kVideoCodecTypeMPEG4; break;
        default: break;
        }
        return kVideoCodecTypeInvalid;
    }
    
    ::AVCodecID FFMpeg::avCodecIdFromVideoCodecType(eVideoCodecType vct)
    {
        switch(vct)
        {
        case kVideoCodecTypeH263:  return ::AV_CODEC_ID_H263; break;
        case kVideoCodecTypeH264:  return ::AV_CODEC_ID_H264; break;
        case kVideoCodecTypeMPEG4: return ::AV_CODEC_ID_MPEG4; break;
        default: break;
        }
        return ::AV_CODEC_ID_NONE;
    }
    
    base::ePixelFormat FFMpeg::pixelFormatFromAVPixelFormat(::AVPixelFormat fmt)
    {
        switch(fmt)
        {
        case ::AV_PIX_FMT_RGB24:        return base::kPixelFormatRGB8; break;
        case ::AV_PIX_FMT_BGRA:         return base::kPixelFormatBGRA8; break;
        case ::AV_PIX_FMT_NV12:         return base::kPixelFormatYUV420sp; break;
        default: break;
        }
        return base::kPixelFormatInvalid;
    }
    
    ::AVPixelFormat FFMpeg::avPixelFormatFromPixelFormat(base::ePixelFormat fmt)
    {
        switch(fmt)
        {
        case base::kPixelFormatRGB8:         return ::AV_PIX_FMT_RGB24; break;
        case base::kPixelFormatBGRA8:        return ::AV_PIX_FMT_BGRA; break;
        case base::kPixelFormatYUV420sp:     return ::AV_PIX_FMT_NV12; break;
        default: break;
        }
        return AV_PIX_FMT_NONE;
    }
    
    eContainerFormat FFMpeg::containerFormatFromFFMpegName(const char *name)
    {
        if(0 == ::strcmp(name, "mp4"))
            return kContainerFormatMPEG4;
        
        if(0 == ::strcmp(name, "mov"))
           return kContainerFormatMOV;
        
        if(0 == ::strcmp(name, "mpegts"))
           return kContainerFormatMPEGTS;
        
        return kContainerFormatInvalid;
    }
    
    int FFMpeg::av_opt_set_flag(void *obj, const char *flag_name, const char *field_name)
    {
        const AVOption *field = av_opt_find(obj, field_name, NULL, 0, 0);
        const AVOption *flag  = av_opt_find(obj, flag_name, field ? field->unit : NULL, 0, 0);
        
        int64_t val;
        if (!field || !flag || flag->type != AV_OPT_TYPE_CONST || av_opt_get_int(obj, field_name, 0, &val) < 0)
            return -1;
        
        val |= flag->default_val.i64;
        return ::av_opt_set_int(obj, field_name, val, 0);
    }
    
    system::error_code FFMpeg::registerVideoEncoders(ComponentFactory &f)
    {
        if(!isInitialized())
			return base::makeErrorCode(base::kENotFound);
        
        for(::AVCodec *codec = ::av_codec_next(0); codec != 0; codec = ::av_codec_next(codec))
        {
            if(codec->type != AVMEDIA_TYPE_VIDEO)
                continue;
            
            if(!::av_codec_is_encoder(codec))
                continue;
            
            f.registerComponent(
                intrusive_ptr<Component::Registry>(
                    new VideoEncoder::Registry(codec->long_name, codec)
                )
            );
        }
        return base::makeErrorCode(base::kENoError);
    }
    
    system::error_code FFMpeg::registerVideoDecoders(ComponentFactory &f)
    {
        if(!isInitialized())
			return base::makeErrorCode(base::kENotFound);
        
        for(::AVCodec *codec = ::av_codec_next(0); codec != 0; codec = ::av_codec_next(codec))
        {
            if(codec->type != AVMEDIA_TYPE_VIDEO)
                continue;
            
            if(!::av_codec_is_decoder(codec))
                continue;
            
            f.registerComponent(
                intrusive_ptr<Component::Registry>(
                    new VideoDecoder::Registry(codec->long_name, codec)
                )
            );
            
        }
		return base::makeErrorCode(base::kENoError);
    }

} //namespace ffmpeg
} //namespace media
} //namespace uquad
