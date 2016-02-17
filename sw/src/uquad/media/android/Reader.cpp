#include "Reader.h"
#include "../ComponentFactory.h"
#include "../../base/Error.h"
#include "../RawReader.h"

namespace uquad
{
namespace media
{
    template<>
    system::error_code Component::registerComponents<Reader>(ComponentFactory &f)
    {
        f.registerComponent(
            intrusive_ptr<Component::Registry>(
                new media::RawReader::Registry()
            )
        );

        return base::makeErrorCode(base::kENoError);
    }

} //namespace media
} //namespace uquad
