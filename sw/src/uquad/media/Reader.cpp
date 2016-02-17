#include "Reader.h"

namespace uquad
{
namespace media
{
    Reader::Registry::Registry(std::string const &n)
        : Component::Registry(n, typeid(Reader))
    {
    }
    
    Reader::Registry::~Registry()
    {
    }
        
	Reader::Reader()
        : Generator()
	{
	}

	Reader::~Reader()
	{
	}

} //namespace media
} //namespace uquad
