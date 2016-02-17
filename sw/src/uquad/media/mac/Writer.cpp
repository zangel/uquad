#include "Writer.h"
#include "../ComponentFactory.h"
#include "../../base/Error.h"
#include "../RawWriter.h"

namespace uquad
{
namespace media
{
	template<>
	system::error_code Component::registerComponents<Writer>(ComponentFactory &f)
	{
        f.registerComponent(
            intrusive_ptr<Component::Registry>(
                new media::RawWriter::Registry()
            )
        );
        
		return base::makeErrorCode(base::kENoError);
	}

} //namespace media
} //namespace uquad
