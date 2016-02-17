#ifndef UQUAD_MEDIA_FFMPEG_FFMPEG_H
#define UQUAD_MEDIA_FFMPEG_FFMPEG_H

#include "../Config.h"
#include "../SampleType.h"
#include "../ContainerFormat.h"
#include "../ComponentFactory.h"

extern "C"
{
    #include <libavfilter/avfilter.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/opt.h>
	#include <libswscale/swscale.h>
   
}


namespace uquad
{
namespace media
{
namespace ffmpeg
{
	class FFMpeg
	{
	public:
	protected:
		FFMpeg();

	public:
		inline bool isInitialized() const { return m_bInitialized; }
        
        static eVideoCodecType videoCodecTypeFromAVCodecId(::AVCodecID cid);
        static ::AVCodecID avCodecIdFromVideoCodecType(eVideoCodecType vct);
        static base::ePixelFormat pixelFormatFromAVPixelFormat(::AVPixelFormat fmt);
        static ::AVPixelFormat avPixelFormatFromPixelFormat(base::ePixelFormat pfmt);
        static eContainerFormat containerFormatFromFFMpegName(const char *name);
        
        static int av_opt_set_flag(void *obj, const char *flag_name, const char *field_name);
        
        system::error_code registerVideoEncoders(ComponentFactory &f);
        system::error_code registerVideoDecoders(ComponentFactory &f);
        
    public:
       ~FFMpeg();

	private:
       bool m_bInitialized;
	};

} //namespace ffmpeg
} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_FFMPEG_FFMPEG_H
