#include "VideoDecoder.h"
#include "../ComponentFactory.h"
#include "../../base/Error.h"
#include "../vt/VT.h"


namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<VideoDecoder>(ComponentFactory &f)
	{
        if(SI(vt::VT).registerVideoDecoders(f))
            return base::makeErrorCode(base::kENotFound);
        
        return base::makeErrorCode(base::kENoError);
        
	}

} //namespace media
} //namespace uquad
