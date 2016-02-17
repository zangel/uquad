#include "VideoDecoder.h"

namespace uquad
{
namespace media
{
    VideoDecoder::Registry::Registry(std::string const &n)
        : VideoCodec::Registry(n, typeid(VideoDecoder))
    {
    }
    
    VideoDecoder::Registry::~Registry()
    {
    }
        
	VideoDecoder::VideoDecoder()
        : VideoCodec()
	{
	}

	VideoDecoder::~VideoDecoder()
	{
	}
    
} //namespace media
} //namespace uquad
