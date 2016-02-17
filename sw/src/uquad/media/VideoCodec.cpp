#include "VideoCodec.h"

namespace uquad
{
namespace media
{
    VideoCodec::Registry::Registry(std::string const &n, Component::Type const &type)
        : Codec::Registry(n, type)
    {
    }
    
    VideoCodec::Registry::~Registry()
    {
    }
        
	VideoCodec::VideoCodec()
        : Codec()
	{
	}

	VideoCodec::~VideoCodec()
	{
	}
    
} //namespace media
} //namespace uquad
