#include "VideoDecoder.h"
#include "../ComponentFactory.h"
#include "../Error.h"
#include "../ffmpeg/FFMpeg.h"
#include "../vt/VT.h"
#include "../ffmpeg/VideoDecoder.h"

namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<VideoDecoder>(ComponentFactory &f)
	{
        if(/*SI(ffmpeg::FFMpeg).registerVideoDecoders(f) || */SI(vt::VT).registerVideoDecoders(f))
            return makeErrorCode(kENotFound);
        
        return makeErrorCode(kENoError);
        
	}

} //namespace media
} //namespace uquad
