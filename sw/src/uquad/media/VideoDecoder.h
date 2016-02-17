#ifndef UQUAD_MEDIA_VIDEO_DECODER_H
#define UQUAD_MEDIA_VIDEO_DECODER_H

#include "Config.h"
#include "VideoCodec.h"

namespace uquad
{
namespace media
{
    class VideoDecoder
    	: public VideoCodec
    {
    public:
        
        class Registry
            : public VideoCodec::Registry
        {
        public:
            Registry(std::string const &n);
            ~Registry();
        };

    	VideoDecoder();
    	~VideoDecoder();
    };

} //namespace media
} //namespace uquad

#endif //UQUAD_MEDIA_VIDEO_DECODER_H
