#include "VideoEncoder.h"
#include "../../base/Error.h"
#include "../vt/VT.h"


namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<VideoEncoder>(ComponentFactory &f)
	{
        if(SI(vt::VT).registerVideoEncoders(f))
            return base::makeErrorCode(base::kENotFound);
        
        return base::makeErrorCode(base::kENoError);
	}

} //namespace media
} //namespace uquad
