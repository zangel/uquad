#include "VideoDecoder.h"
#include "../../base/Error.h"
#include "../omx/OMXCore.h"

namespace uquad
{
namespace media
{
    template<>
    system::error_code Component::registerComponents<VideoDecoder>(ComponentFactory &f)
    {
        if(SI(omx::OMXCore).registerVideoDecoders(f))
            return base::makeErrorCode(base::kENotFound);

        return base::makeErrorCode(base::kENoError);
    }

} //namespace media
} //namespace uquad
