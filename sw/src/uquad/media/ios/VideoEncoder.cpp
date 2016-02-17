#include "VideoEncoder.h"
#include "../Error.h"
#include "../vt/VT.h"
#include "../ffmpeg/FFMpeg.h"


namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<VideoEncoder>(ComponentFactory &f)
	{
        if(SI(vt::VT).registerVideoEncoders(f) /*|| SI(ffmpeg::FFMpeg).registerVideoEncoders(f)*/)
            return makeErrorCode(kENotFound);
        
        return makeErrorCode(kENoError);
	}

} //namespace media
} //namespace uquad
