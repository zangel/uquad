#include "UQuad.h"
#include "../DeviceManager.h"
#include "../../base/Error.h"

namespace uquad
{
namespace hal
{

    template<>
    system::error_code Device::registerDevices<UQuad>(DeviceManager &dm)
    {
        return base::makeErrorCode(base::kENoError);
    }
    
} //namespace hal
} //namespace uquad
