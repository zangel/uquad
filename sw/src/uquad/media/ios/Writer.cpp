#include "Writer.h"
#include "../ComponentFactory.h"
#include "../Error.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "../ffmpeg/FFMpeg.h"
#include "../RawWriter.h"

//#include "../ffmpeg/Writer.h"

namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<Writer>(ComponentFactory &f)
	{
		if(!SI(ffmpeg::FFMpeg).isInitialized())
			return makeErrorCode(kENotFound);
        
        f.registerComponent(
            intrusive_ptr<Component::Registry>(
                new media::RawWriter::Registry()
            )
        );
        
        for(::AVOutputFormat *format = ::av_oformat_next(0); format != 0; format = ::av_oformat_next(format))
        {
            if(format->video_codec == AV_CODEC_ID_NONE)
                continue;
            
            //if(format->audio_codec != AV_CODEC_ID_NONE)
            //    continue;
            /*
            f.registerComponent(
                intrusive_ptr<Component::Registry>(
                    new ffmpeg::Writer::Registry(format->long_name, format)
                )
            );
            */
        }
		return makeErrorCode(kENoError);
	}

} //namespace media
} //namespace uquad
