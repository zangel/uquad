#include "Writer.h"

namespace uquad
{
namespace media
{
    Writer::Registry::Registry(std::string const &n)
        : Component::Registry(n, typeid(Writer))
    {
    }
    
    Writer::Registry::~Registry()
    {
    }
        
	Writer::Writer()
        : Consumer()
	{
	}

	Writer::~Writer()
	{
	}

} //namespace media
} //namespace uquad
