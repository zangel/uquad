#include "VideoEncoder.h"

namespace uquad
{
namespace media
{
    VideoEncoder::Registry::Registry(std::string const &n)
        : VideoCodec::Registry(n, typeid(VideoEncoder))
    {
    }
    
    VideoEncoder::Registry::~Registry()
    {
    }
        
	VideoEncoder::VideoEncoder()
        : VideoCodec()
	{
	}

	VideoEncoder::~VideoEncoder()
	{
	}
    
} //namespace media
} //namespace uquad
